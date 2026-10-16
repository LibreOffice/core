"""Image metrics shared by the scoring passes.

Grayscale, numpy, no third-party imaging beyond Pillow for decode. The metric set
follows the render-comparison skill: no single number separates "a shape is
missing" from "everything moved down three pixels", so several are kept.
"""
from __future__ import annotations

import numpy as np
from PIL import Image

TILE = 32
DIFF_TOLERANCE = 12      # per-channel 0-255 difference treated as antialiasing
INK_THRESHOLD = 250      # luma below this counts as ink
SHIFT_SEARCH = 40


def to_gray(img: Image.Image) -> np.ndarray:
    """Rec. 601 luma. Averaging the channels instead would score saturated yellow
    as ink and a yellow-to-periwinkle gradient as a match; the skill records that
    exact false negative."""
    if img.mode not in ("L", "RGB"):
        img = img.convert("RGB")
    if img.mode == "RGB":
        img = img.convert("L")
    return np.asarray(img, dtype=np.uint8)


def resize_to(gray: np.ndarray, shape: tuple[int, int]) -> np.ndarray:
    h, w = shape
    if gray.shape == (h, w):
        return gray
    return np.asarray(Image.fromarray(gray).resize((w, h), Image.LANCZOS), dtype=np.uint8)


def _boxfilter(a: np.ndarray, r: int) -> np.ndarray:
    """Mean over a (2r+1)^2 window, edges handled by shrinking the window."""
    pad = np.cumsum(np.cumsum(a, axis=0), axis=1)
    pad = np.pad(pad, ((1, 0), (1, 0)))
    h, w = a.shape
    ys = np.arange(h)
    xs = np.arange(w)
    y0 = np.clip(ys - r, 0, h)[:, None]
    y1 = np.clip(ys + r + 1, 0, h)[:, None]
    x0 = np.clip(xs - r, 0, w)[None, :]
    x1 = np.clip(xs + r + 1, 0, w)[None, :]
    total = pad[y1, x1] - pad[y0, x1] - pad[y1, x0] + pad[y0, x0]
    count = (y1 - y0) * (x1 - x0)
    return total / count


def ssim(a: np.ndarray, b: np.ndarray, radius: int = 5) -> float:
    """Mean SSIM over the page. Tolerant of antialiasing, sensitive to structure --
    the single number used as the headline 'closeness', with the diagnostic
    metrics below reported alongside it so it is never read on its own."""
    x = a.astype(np.float64)
    y = b.astype(np.float64)
    c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
    mx, my = _boxfilter(x, radius), _boxfilter(y, radius)
    mxx, myy, mxy = _boxfilter(x * x, radius), _boxfilter(y * y, radius), _boxfilter(x * y, radius)
    vx, vy = mxx - mx * mx, myy - my * my
    cxy = mxy - mx * my
    num = (2 * mx * my + c1) * (2 * cxy + c2)
    den = (mx * mx + my * my + c1) * (vx + vy + c2)
    return float(np.mean(num / den))


def ink_fraction(g: np.ndarray) -> float:
    return float(np.count_nonzero(g < INK_THRESHOLD)) / g.size


def row_profile_shift(a: np.ndarray, b: np.ndarray) -> int:
    """Vertical offset that best aligns the two ink-per-row profiles. Non-zero is
    strong evidence the content is present but moved."""
    pa = (a < INK_THRESHOLD).sum(axis=1).astype(np.float64)
    pb = (b < INK_THRESHOLD).sum(axis=1).astype(np.float64)
    n = min(len(pa), len(pb))
    if n == 0:
        return 0
    limit = min(SHIFT_SEARCH, max(1, n // 4))
    best, best_cost = 0, None
    for s in range(-limit, limit + 1):
        if s >= 0:
            cost = np.abs(pa[: n - s] - pb[s:n]).mean()
        else:
            cost = np.abs(pa[-s:n] - pb[: n + s]).mean()
        if best_cost is None or cost < best_cost:
            best, best_cost = s, cost
    return best


def tile_metrics(a: np.ndarray, b: np.ndarray) -> tuple[float, int, int]:
    """(worst tile mean error, tiles differing, tiles that merely moved).

    A tile counts as shifted when it matches poorly in place but well at a small
    vertical offset -- the reflow-cascade signature."""
    h, w = a.shape
    th, tw = (h + TILE - 1) // TILE, (w + TILE - 1) // TILE
    ph, pw = th * TILE, tw * TILE
    pa = np.full((ph, pw), 255, np.uint8); pa[:h, :w] = a
    pb = np.full((ph, pw), 255, np.uint8); pb[:h, :w] = b
    d = np.abs(pa.astype(np.int16) - pb.astype(np.int16))
    base = d.reshape(th, TILE, tw, TILE).mean(axis=(1, 3)) / 255.0
    worst = float(base.max()) if base.size else 0.0
    differing = base > 0.02
    n_diff = int(differing.sum())
    shifted = np.zeros_like(differing)
    for dy in (-8, -4, -2, -1, 1, 2, 4, 8):
        rolled = np.roll(pb, -dy, axis=0)
        alt = (np.abs(pa.astype(np.int16) - rolled.astype(np.int16))
               .reshape(th, TILE, tw, TILE).mean(axis=(1, 3)) / 255.0)
        shifted |= differing & (alt < base * 0.4)
    return worst, n_diff, int((shifted & differing).sum())


def compare(actual: np.ndarray, expected: np.ndarray) -> dict:
    """Compare two same-sized grayscale pages."""
    d = np.abs(actual.astype(np.int16) - expected.astype(np.int16))
    worst_tile, diff_tiles, shifted = tile_metrics(actual, expected)
    ia, ie = ink_fraction(actual), ink_fraction(expected)
    ratio = (ia / ie) if ie > 1e-9 else (1.0 if ia <= 1e-9 else float("inf"))
    n_tiles = max(1, ((actual.shape[0] + TILE - 1) // TILE)
                     * ((actual.shape[1] + TILE - 1) // TILE))
    return {
        "ssim": round(ssim(actual, expected), 5),
        "differing_fraction": round(float((d > DIFF_TOLERANCE).mean()), 5),
        "mean_abs_error": round(float(d.mean()) / 255.0, 5),
        "max_tile_error": round(worst_tile, 5),
        "differing_tiles": diff_tiles,
        "shifted_tiles": shifted,
        "tiles": n_tiles,
        "ink_actual": round(ia, 5),
        "ink_expected": round(ie, 5),
        "ink_ratio": round(min(ratio, 99.0), 4),
        "row_profile_shift": row_profile_shift(actual, expected),
    }


def diagnose(m: dict) -> str:
    """The numbers turned into the likely cause (render-comparison SKILL.md)."""
    if (m["differing_fraction"] < 0.005 and m["mean_abs_error"] < 0.004
            and m["max_tile_error"] < 0.05 and 0.98 <= m["ink_ratio"] <= 1.02):
        return "match"
    # Ink ratio is meaningless on a nearly blank page: a handful of antialiased
    # pixels moves it by tens of percent. Require a real amount of ink first.
    inked = m["ink_expected"] > 0.002
    if inked and m["ink_ratio"] < 0.92:
        return "content-missing"
    if inked and m["ink_ratio"] > 1.08:
        return "extra-content"
    if m["shifted_tiles"] >= 3 or abs(m["row_profile_shift"]) > 2:
        return "reflow-cascade"
    if m["max_tile_error"] > 0.15 and m["mean_abs_error"] < 0.02:
        return "localised"
    return "differs"
