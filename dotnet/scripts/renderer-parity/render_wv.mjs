// Drive the Rust/WASM viewer in headless Chromium and capture each page as a PNG.
//
// One Vite dev server per family (the packages are separate Vite roots), one
// browser, a fresh page per document so a hung or crashed load cannot poison
// the next one.
import { chromium } from '/home/user/office-open-xml-viewer/node_modules/.pnpm/playwright@1.59.1/node_modules/playwright/index.mjs';
import { spawn } from 'node:child_process';
import { mkdirSync, readFileSync, writeFileSync, existsSync } from 'node:fs';
import path from 'node:path';

const REPO = '/home/user/office-open-xml-viewer';
const OUT = '/data/bench/wv';
const DPI = 150;
const MAX_PAGES = Number(process.env.MAX_PAGES ?? 5);
const LOAD_TIMEOUT = Number(process.env.LOAD_TIMEOUT ?? 90_000);
const RENDER_TIMEOUT = Number(process.env.RENDER_TIMEOUT ?? 90_000);
const DOC_TIMEOUT = Number(process.env.DOC_TIMEOUT ?? 240_000);
const CONCURRENCY = Number(process.env.CONCURRENCY ?? 3);
const CHROME = '/opt/pw-browsers/chromium-1194/chrome-linux/chrome';

const FAMILY_PKG = { docs: 'docx', sheets: 'xlsx', slides: 'pptx' };
const PORT = { docx: 5301, xlsx: 5302, pptx: 5303 };
const ONLY = process.env.ONLY_FAMILY ? process.env.ONLY_FAMILY.split(',') : null;
const LIMIT = process.env.LIMIT ? Number(process.env.LIMIT) : Infinity;
const ONLY_EXT = process.env.ONLY_EXT ? process.env.ONLY_EXT.split(',') : null;

function manifest() {
  const lines = readFileSync('/data/bench/manifest.tsv', 'utf8').trim().split('\n');
  const head = lines[0].split('\t');
  return lines.slice(1).map((l) => Object.fromEntries(l.split('\t').map((v, i) => [head[i], v])));
}

async function startVite(pkg) {
  const port = PORT[pkg];
  const proc = spawn('pnpm', ['exec', 'vite', '--host', '127.0.0.1', '--port', String(port),
                              '--strictPort'],
    { cwd: path.join(REPO, 'packages', pkg), stdio: ['ignore', 'pipe', 'pipe'] });
  proc.stdout.on('data', () => {});
  proc.stderr.on('data', (d) => process.stderr.write(`[vite ${pkg}] ${d}`));
  const url = `http://127.0.0.1:${port}/bench/fixture.html`;
  for (let i = 0; i < 120; i++) {
    try {
      const r = await fetch(url);
      if (r.ok) return { proc, port };
    } catch { /* not up yet */ }
    await new Promise((r) => setTimeout(r, 500));
  }
  throw new Error(`vite for ${pkg} did not come up`);
}

async function captureDoc(browser, pkg, row) {
  const dest = path.join(OUT, row.id);
  const donefile = path.join(dest, 'result.json');
  if (existsSync(donefile)) return JSON.parse(readFileSync(donefile, 'utf8'));
  mkdirSync(dest, { recursive: true });

  const url = `http://127.0.0.1:${PORT[pkg]}/bench/fixture.html`
            + `?id=${encodeURIComponent(row.id)}&dpi=${DPI}`;
  const result = { id: row.id, status: 'failed', pages: 0, captured: 0, error: '' };
  const ctx = await browser.newContext({ viewport: { width: 1280, height: 900 },
                                         deviceScaleFactor: 1 });
  const page = await ctx.newPage();
  try {
    await page.goto(url, { waitUntil: 'commit', timeout: LOAD_TIMEOUT });
    await page.waitForFunction(() => document.body.dataset.status === 'ready'
                                  || document.body.dataset.status === 'error',
                               null, { timeout: LOAD_TIMEOUT });
    if (await page.evaluate(() => document.body.dataset.status) === 'error') {
      result.status = 'load-error';
      result.error = String(await page.evaluate(() => document.body.dataset.errorMessage)).slice(0, 300);
    } else {
      const total = Number(await page.evaluate(() => document.body.dataset.pageCount));
      result.pages = Number.isFinite(total) ? total : 0;
      const n = Math.min(result.pages, MAX_PAGES);
      for (let i = 0; i < n; i++) {
        const geom = await Promise.race([
          page.evaluate((idx) => window.benchRender(idx), i),
          new Promise((_, rej) =>
            setTimeout(() => rej(new Error('render timeout')), RENDER_TIMEOUT)),
        ]);
        // The canvas is routinely taller than any viewport, and a viewport-clipped
        // screenshot would silently capture only its top. Grow the viewport to the
        // canvas, then shoot the element itself.
        await page.setViewportSize({ width: Math.min(4000, geom.width),
                                     height: Math.min(6000, geom.height) });
        await page.locator('#canvas').screenshot({ path: path.join(dest, `page-${i + 1}.png`) });
        if (i === 0) { result.headerW = geom.headerW ?? 0; result.headerH = geom.headerH ?? 0; }
        result.captured++;
      }
      result.status = result.captured > 0 ? 'ok' : 'no-pages';
    }
  } catch (e) {
    result.status = result.captured > 0 ? 'partial' : 'failed';
    result.error = String(e && e.message ? e.message : e).slice(0, 300);
  } finally {
    // Force the target down rather than asking politely: close() waits on the
    // page's JS, and a renderer spinning in a long layout is exactly what has hung.
    await Promise.race([page.close({ runBeforeUnload: false }).catch(() => {}),
                        new Promise((r) => setTimeout(r, 10_000))]);
    await Promise.race([ctx.close().catch(() => {}),
                        new Promise((r) => setTimeout(r, 10_000))]);
  }
  writeFileSync(donefile, JSON.stringify(result));
  return result;
}

/** A hard per-document deadline. Two DOCX in the corpus spin in the viewer's
 *  layout for tens of minutes; no Playwright timeout interrupts a busy renderer,
 *  so the pool has to abandon the document and move on. */
async function captureDocGuarded(browser, pkg, row) {
  let timer;
  const bail = new Promise((resolve) => {
    timer = setTimeout(() => resolve({ id: row.id, status: 'timeout', pages: 0,
                                       captured: 0, error: 'document deadline' }),
                       DOC_TIMEOUT);
  });
  const r = await Promise.race([captureDoc(browser, pkg, row), bail]);
  clearTimeout(timer);
  if (r.status === 'timeout') {
    const dest = path.join(OUT, row.id);
    mkdirSync(dest, { recursive: true });
    writeFileSync(path.join(dest, 'result.json'), JSON.stringify(r));
  }
  return r;
}

function launch() {
  return chromium.launch({ executablePath: CHROME,
    args: ['--disable-gpu', '--no-sandbox', '--font-render-hinting=none'] });
}

/** Abandoning a document at its deadline leaves its context behind -- there is no
 *  way to reclaim one whose renderer will not yield -- and enough of those kill the
 *  browser. Replace it when it dies rather than on a count: recycling on a count
 *  would close a browser the other two workers are still holding pages in. */
class BrowserPool {
  async get() {
    if (this.browser && this.browser.isConnected()) return this.browser;
    if (this.pending) return this.pending;
    this.pending = (async () => {
      if (this.browser) await this.browser.close().catch(() => {});
      this.browser = await launch();
      this.pending = null;
      return this.browser;
    })();
    return this.pending;
  }
  lease() {}
  async close() { if (this.browser) await this.browser.close().catch(() => {}); }
}

async function main() {
  mkdirSync(OUT, { recursive: true });
  const rows = manifest();
  const pool = new BrowserPool();
  const summary = {};
  for (const [family, pkg] of Object.entries(FAMILY_PKG)) {
    if (ONLY && !ONLY.includes(family)) continue;
    const mine = rows.filter((r) => r.family === family && (!ONLY_EXT || ONLY_EXT.includes(r.ext))).slice(0, LIMIT);
    if (!mine.length) continue;
    console.log(`=== ${family} (${pkg}): ${mine.length} documents`);
    const { proc } = await startVite(pkg);
    const t0 = Date.now();
    try {
      // A small pool: each document gets its own browser context, so one hung
      // load cannot stall the others, and three at a time keeps four cores busy
      // without starving the Vite server that feeds them.
      let next = 0, finished = 0;
      const worker = async () => {
        for (;;) {
          const i = next++;
          if (i >= mine.length) return;
          let r;
          try {
            const browser = await pool.get();
            pool.lease();
            r = await captureDocGuarded(browser, pkg, mine[i]);
          } catch (e) {
            // A dead browser must not end the sweep: record the document and go on.
            r = { id: mine[i].id, status: 'harness-error', pages: 0, captured: 0,
                  error: String(e && e.message ? e.message : e).slice(0, 200) };
            writeFileSync(path.join(OUT, mine[i].id, 'result.json'), JSON.stringify(r));
          }
          summary[r.id] = r;
          finished++;
          if (finished % 20 === 0 || finished === mine.length) {
            const el = (Date.now() - t0) / 1000;
            console.log(`${family} ${finished}/${mine.length}  ${el.toFixed(0)}s, ` +
                        `eta ${(el / finished * (mine.length - finished)).toFixed(0)}s`);
          }
        }
      };
      await Promise.all(Array.from({ length: CONCURRENCY }, worker));
    } finally {
      proc.kill('SIGTERM');
    }
  }
  await pool.close();
  const prev = existsSync('/data/bench/wv-status.json')
    ? JSON.parse(readFileSync('/data/bench/wv-status.json', 'utf8')) : {};
  writeFileSync('/data/bench/wv-status.json', JSON.stringify({ ...prev, ...summary }, null, 1));
  const counts = {};
  for (const v of Object.values(summary)) counts[v.status] = (counts[v.status] ?? 0) + 1;
  console.log(counts);
}

await main();
