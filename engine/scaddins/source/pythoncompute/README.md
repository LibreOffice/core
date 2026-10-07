# Calc =PY() / =PYTHON() — scaddins pythoncompute

Thin C++ Calc AddIn that returns `XVolatileResult` with interim **`#BUSY!`**
(string — Core has no `FormulaError::Busy`), then queues dumb JSON for the Online kit, which sends it to a remote Python
compute service. Without a kit that brokers =PY() (it sets `COOL_PYTHON_COMPUTE`; not on
desktop, mobile, or an Online built without `--enable-python-compute`) the cell is
`#DISABLED` at once and nothing is queued.

## Files

| File | Role |
|------|------|
| `addin.*` | `com.sun.star.sheet.AddIn` — `org.collaboraoffice.sheet.addin.PythonComputeFunctions` (`getPy` / `getPython`) |
| `volatile.*` | `XVolatileResult` holder |
| `anyjson.*` | Calc `Any` ↔ dumb JSON via `tools::JsonWriter` (emit) + a small typed hand parser; no NumPy |
| `bridge.*` / `.h` | Pending map, param cache, outbox + C API (`pythoncompute_set_wakeup`, `pythoncompute_take_requests`, `pythoncompute_complete`) |

## Wire

```
=PY(code; data?) → AddIn → XVolatileResult("#BUSY!") + request in outbox → wakeup
                 → (kit poll) take_requests → pythoncompute: frame
                 → (wsd) POST → compute service
                 → pythoncomputeresult: (document-level) → complete → ResultEvent
```

The kit finds the C API with `dlsym` (globally, or in the AddIn library for non-merged builds;
`kit/PythonComputeEmitter.cpp`) on each input batch until found, installs the wakeup then, and
drains the outbox after load and on each new session, so on-load recalcs are sent once a view
exists.

**No timeouts here.** coolwsd answers every request it accepts exactly once: the result, an
error, or its own HTTP timeout (`timeout_secs`). If the kit or wsd goes away, the document
goes with it. So a pending cell needs no clock of its own.

## Locking

The whole lock graph is SolarMutex → kit poll callback queue (leaf). AddIn state lives in one
leaked `BridgeState` guarded by SolarMutex only. Under Solar the AddIn only queues and calls the
wakeup, which just queues a poll callback. `take_requests` moves the outbox out under Solar,
**releases** it, and only then calls the kit callback that writes the socket. `complete` and
`finish()` take Solar; listeners run last, after all state is updated, from snapshots.

## Correctness notes

**Identity:** Same `(code, data)` returns the **same** `XVolatileResult` (AddIn.idl). A cache hit
does not queue a new request. Cache keys are SHA-256 of the request JSON with a fixed id.

**Cache entries** are `{weak volatile, Pending|Ok|Error, retryAfter}`. Results and errors raised
by the user's code, and replies that cannot be parsed, are cached like `Ok` (change the args to
retry). Transient errors (coolwsd marks them `"transient":true`: transport, timeout, too large,
disabled) become
`Error` with `retryAfter = now + 30 s`
(`kRetryAfterMs`): the next recalc after that sends again, so there is no retry storm. Entries are
never erased on error. A pending entry's key is never reused (its volatile is held and its state
is not `Error`). Each time the cache has doubled, lapsed entries are pruned; live ones are never evicted. Calc's `ScAddInListener` keeps volatiles alive until the document closes.

**Limits:** one admin setting, `security.python_compute.max_payload_bytes` (default 16 MiB, also
for 0 or less; passed in by the kit), scales them all. A request over it is `#VALUE!` at once
(transient, retried after 30 s). Flow control, not failure: the kit is handed requests while
those it holds without a result total at most 16 × `max_payload_bytes` (`kHandedOffPayloads`;
always at least one); the rest stay `#BUSY!` in the outbox and go, oldest first, as results come.
That also bounds coolwsd's per-document queue (behind its 8 in-flight POSTs). Backstop for kit
memory: past 64 × `max_payload_bytes`, at least 1 GiB, of pending request JSON (`kHeldPayloads`), a new request is
`#N/A` (transient); the only pending request is never refused.

**Errors:** Failures finish as formula errors. Detail text is for logs only.

| Failure | Cell |
|---------|------|
| Timeout / transient wsd error | `#VALUE!` (retried after 30 s) |
| Service `status=error` / bad or missing JSON result | `#VALUE!` |
| Disabled by coolwsd, or no Online kit (desktop) | `#DISABLED` string |
| Over the held-bytes backstop | `#N/A` (retried after 30 s) |
| Interim pending | literal `#BUSY!` string |
| Plot-not-supported / ok `null` | informational / empty string |

**JSON:** Dumb scalar/array `Any` ↔ JSON only (no full JSON object-as-cell model).
Rectangular numeric 2D arrays (including N×1 columns) become `sequence<sequence<double>>`
for Calc matrix formulas. Bare JSON numbers stay scalars; one-element lists are 1×1 matrices.
Single-cell formulas currently keep only the matrix's top-left value; automatic spill is deferred.

**JSON depth:** service JSON nested deeper than `kMaxJsonDepth` (64) is rejected and cached as `#VALUE!`.

## Build notes

`Library_pythoncompute.mk` links `tl` (JsonWriter), `vcl` (Application / Solar), and `comphelper` (hash, LOK check).
Online coolwsd keeps using `Poco::JSON` / `JsonUtil` on the broker side — do not pull Poco into
this AddIn. The AddIn is on by default; omit it with `--disable-python-compute`.

## Debugging

There are **two** log systems; the browser console will not show either.

**coolwsd / coolkit** (`LOG_INF` / `LOG_DBG` / `LOG_WRN`): set `coolwsd.xml`
`<logging><level>trace</level>` (or use `make run`). Grep logs for `pythoncompute` /
`Python compute`.

**Core AddIn inside kit** (`SAL_INFO("scaddins.pythoncompute", …)`):

```bash
export SAL_LOG=+INFO.scaddins.pythoncompute
```

Or after connect, protocol command `sallogoverride +INFO.scaddins.pythoncompute`.
Note: non-debug coolwsd may set `logging.lokit_sal_log` to `-INFO-WARN` by default — override
`SAL_LOG` (or that config) so Info lines from this area are not stripped.

Also confirm `security.python_compute.enable` is true in the **running** config and the compute
service answers `GET /health`.

## coolwsd config (`security.python_compute`)

| Key | Default | Role |
|-----|---------|------|
| `enable` | `false` | Gate remote compute |
| `url` | `http://localhost:8000/v1/execute` | POST endpoint |
| `api_key` | empty | Optional `Authorization: Bearer …` (omit header when empty) |
| `timeout_secs` | `60` | HTTP timeout (no automatic retries) |
| `max_payload_bytes` | `16777216` | Largest request and response body |
