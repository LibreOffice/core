# Clipboard handling across CODA, mobile and COOL

Status snapshot of how copy and paste work in CODA, the mobile apps, the
WASM build, and COOL today, with code pointers. File paths are relative
to the monorepo root. The goal is to show what each platform does, what
they share, and where browser and native builds fail to interoperate at
full fidelity. The document ends with an Action plan.

Terms used here: UTI is the macOS Uniform Type Identifier. Kit means
the document engine process (the local LibreOffice-derived renderer).
CODA is the Collabora Office Desktop App (the macOS, Windows and Qt apps).
The iOS and Android apps are the mobile apps. They are not CODA, but they
are built the same way, so we group them with CODA wherever the clipboard
logic is shared.

## The two clipboard architectures

Everything below is one of two shapes.

- **Web (coolwsd server).** The real, full-fidelity selection lives
  server-side inside the Kit. The browser never holds the rich bytes. It
  holds a small marker plus, for small selections, inline HTML and plain
  text. Rich content is pulled over HTTP from a tokenised endpoint.
  - Consequence: copy and paste between two COOL documents keep full
    fidelity, because the bytes never have to fit through the browser
    clipboard. The marker on the operating system clipboard points the
    paste back at the server-held transferable.
  - Consequence: the moment either end is not COOL (an external app, or a
    native app reading the same operating system clipboard), fidelity
    drops to whatever the browser could publish, which is HTML, plain
    text, and PNG. Browsers do not let JavaScript put custom MIME types
    such as `application/x-openoffice-embed-source-xml` on the clipboard.
- **CODA and mobile and WASM (MOBILEAPP builds).** The Kit runs
  locally (in process for the apps, in the browser tab for WASM). There
  is no coolwsd HTTP clipboard endpoint. A native glue layer copies bytes
  between the operating system clipboard and the local Kit. The whole
  server HTTP clipboard path is compiled out under `#if !MOBILEAPP`.
  - Consequence: the desktop and mobile apps reach the operating system
    clipboard through native code (NSPasteboard, Win32, QClipboard,
    UIPasteboard, Android ClipboardManager), so they can publish and read
    the full native flavor set. App to app copy on the same operating
    system keeps full fidelity.
  - Consequence: there is no server token and no `/cool/clipboard` GET, so
    the cross-server paste trick (scenario 3 below) does not exist here.

WASM is the odd member of the second group, and the least capable for the
clipboard. It is a MOBILEAPP build, so the server endpoint is gone, but it
ships no native bridge code either. The browser JavaScript talks to the
in-tab Kit over an in-process fake WebSocket, but the only door to the
operating system clipboard is the browser `navigator.clipboard` API.

What WASM can copy and paste:
- It can put HTML, plain text, and PNG on the operating system clipboard,
  and read the same back. That is the browser ceiling, the same limit the
  web client hits when the other end is not COOL.
- It cannot publish or read the rich native flavors (ODF, Star Embed
  Source XML, Object Descriptor). The browser sandbox hides those MIME
  types from JavaScript.
- It has no server-held transferable to fall back on. In the web
  deployment `getMetaBase()` returns a real HTTP origin, but for any
  MOBILEAPP build it returns the synthetic string
  `'collabora-online-mobile'` (`browser/src/map/Clipboard.js:140`). The
  desktop and mobile apps never notice, because they branch into native
  code before using that URL (`Clipboard.js:941,967,976`). WASM does not
  branch, so it falls through to `fetch(getMetaURL()...)`
  (`Clipboard.js:982,990`) against a URL with no server behind it. The
  marker is therefore only an own-content identity check for WASM, not a
  fetchable source.
- Net result: inside one WASM document the in-tab Kit still keeps the real
  transferable, so a copy and paste within the same document is full
  fidelity (the paste issues only `.uno:Paste`). Anything that has to
  cross the operating system clipboard is bounded by HTML, plain text, and
  PNG.

### Two worked interactions

Copy a styled paragraph in the web client and paste it back into the same
document (full fidelity, server-held):
1. Ctrl+C. `oncopy` reaches `copy` then `_doCopyCut`
   (`Clipboard.js:1267`), which calls `_navigatorClipboardWrite` then
   `_asyncAttemptNavigatorClipboardWrite` (`Clipboard.js:938`).
2. The browser sends `.uno:Copy` over the WebSocket and waits for the
   command to finish. The Kit runs Copy and keeps the real transferable in
   its per-view `KitClipboard`. The server hands back a clipboard token in
   `clipboardkey:`.
3. The browser does `GET /cool/clipboard?...&Tag=<token>&MimeType=text/html,text/plain`
   (`Clipboard.js:982`). The server answers from the Kit through
   `handleClipboardRequest` (`wsd/ClientRequestDispatcher.cpp:1774`).
4. The browser writes a `ClipboardItem` with `text/html` (wrapped in the
   meta-origin div) and `text/plain` to `navigator.clipboard`. The
   operating system clipboard now holds only those two formats. The ODF
   and embed-source bytes stay on the server.
5. Ctrl+V in the same document. `paste` reads the HTML, and
   `dataTransferToDocument` (`Clipboard.js:466`) sees the meta-origin
   matches this view's token, so it issues only `.uno:Paste`
   (`_doInternalPaste`). The Kit pastes from its retained transferable, so
   the result is full fidelity, not the reduced HTML that was on the
   operating system clipboard.

Copy in the macOS CODA app and paste into a plain text editor, then back
(full native fidelity, no server). macOS now runs through the engine-driven
COKitClipboardProvider (see the Done section), so the engine decides the
formats and the app does only raw NSPasteboard input and output:
1. Ctrl+C. The browser sends `.uno:Copy` over the fake socket. There is no
   `write` script message any more.
2. The Kit runs Copy and sets its per-view clipboard. Because the macOS app
   registered a provider, the Kit calls the provider advertise callback
   directly, in process, with its flavor list. The macOS side advertises
   those flavors on NSPasteboard with a retained owner (COClipboardOwner)
   and no bytes (HTML, RTF, PNG, Star Embed Source XML, Object Descriptor).
3. The text editor asks the pasteboard for the richest flavor it
   understands, for example RTF. The owner fetches just that one format
   from the Kit on demand and hands it over, so the paste keeps formatting.
4. Copy in the text editor, then Ctrl+V in CODA. The browser sends the
   `read` message, which only makes sure the provider is registered. On
   `.uno:Paste` the Kit reads the pasteboard through the provider: it lists
   the flavors, picks the single one its paste path wants, and fetches only
   that. Fidelity is whatever the text editor offered.

## Shared contract (the parts every platform reuses)

- **Kit clipboard API.** `getClipboard`, `setClipboard`, `paste`,
  `getTextSelection`, `setTextSelection`. Declared in
  `engine/include/COKit/COKit.h:456,464,289,284,278`. Implemented in
  `engine/desktop/source/lib/init.cxx:6400,6488,6522,6269,5971`. The
  clipboard is reimplemented for headless use as a per-view `KitClipboard`
  and `KitTransferable` in
  `engine/desktop/source/lib/kitclipboard.cxx` rather than the full UNO
  clipboard plumbing.
- **Wire serialization.** The mimetype, hex-length, raw-bytes tuple
  format that carries multiple flavors in one blob lives in
  `common/ClipboardData.hpp:23`. A payload that begins with
  `<!DOCTYPE html>` is the single-format HTML shortcut and skips the tuple
  parsing.
- **Browser JavaScript module.** `browser/src/map/Clipboard.js` is the
  single client module for every platform: web, the CODA desktop apps,
  the mobile apps, and WASM. It branches on `window.ThisIsTheWindowsApp`,
  `ThisIsTheQtApp`, `ThisIsTheiOSApp`, `ThisIsTheMacOSApp`,
  `ThisIsTheAndroidApp`, `ThisIsTheEmscriptenApp`. Android short-circuits
  most of it and asserts the module is not needed
  (`Clipboard.js:1651`).
- **Own-content marker.** Copied HTML is wrapped in
  `<div id="meta-origin" data-coolorigin="<clipboard endpoint URL>">`.
  The browser adds it in `_originWrapBody` (`Clipboard.js:179`). The
  server adds and strips it in `postProcessCopyPayload` and
  `preProcessSetClipboardPayload`
  (`wsd/ClientSession.cpp:2638,4187`). On paste the browser reads it
  back in `_getMetaOrigin` (`Clipboard.js:216`) to decide internal versus
  external. Android does not use this marker. It uses its own
  `cool-clip-magic-<instance id>` signature instead
  (`android/lib/.../LOActivity.java:107`).

## Per-platform bridge at a glance

| Platform | Bridge mechanism | OS clipboard API | Mapping table |
|---|---|---|---|
| Web | HTTP `/cool/clipboard` plus WebSocket | browser `navigator.clipboard` | `Clipboard.js` |
| macOS | engine-driven COKitClipboardProvider, plus a thin `clipboard` handler (read/sendToInternal) | NSPasteboard, UTType | `macos/coda/coda/COWrapper.mm` |
| Windows | WebView2 `postMobileMessage`: COPY/CUT/PASTE/CLIPBOARD* | Win32 clipboard formats | `windows/coda/CODA/CODA.cpp:631` |
| Qt | QWebChannel `Bridge.cool()`: COPY/CUT/PASTE/... | QClipboard, QMimeData | `qt/QtClipboard.cpp:162` |
| iOS | WKWebView `clipboard` handler: read/write/sendToInternal | UIPasteboard, UTType | `ios/Mobile/DocumentViewController.mm:307` |
| Android | `@JavascriptInterface` plus JNI getClipboardContent/setClipboardContent/paste | ClipboardManager, ClipData | `android/lib/.../androidapp.cpp:355` |
| WASM | none, uses the web browser path | browser `navigator.clipboard` | `Clipboard.js` |

A plain-text-only fast path exists as `TEXTCLIPBOARD <text>` on macOS,
Windows, and Qt
(`ViewController.swift:461`, `CODA.cpp:2269`, `Bridge.cpp:582`).

## Copy and paste scenarios

Fidelity ladder from best to worst: native flavors (Star Embed Source
XML, Object Descriptor, ODF) > HTML > RTF > plain text.

### 1. Inside one document, same view (web or any app)

- Highest fidelity. The Kit keeps the real transferable per view. Paste
  detects own content and issues only `.uno:Paste`. Nothing round-trips
  through serialization.
- Web decision in `dataTransferToDocument` (`Clipboard.js:466`) matches
  the pasted meta-origin against this view's current or previous token
  path and short-circuits to `_doInternalPaste` (`Clipboard.js:476`).
- macOS, Windows, Qt do the same with an own-clipboard check so they skip
  re-reading the operating system clipboard: changeCount on macOS
  (`COWrapper.mm:279`), `GetClipboardSequenceNumber` on Windows
  (`CODA.cpp:734`), `ownsClipboard` plus a source document id on Qt
  (`Bridge.cpp:615`).

### 2. Between two documents in the same web server

- Full fidelity. The selection content is held server-side. The browser
  copies a marker plus stub. On paste into the second document the meta
  origin matches this server, so the browser issues `.uno:Paste` and the
  Kit reads the cached clipboard. Cache lookup for disconnected views is
  `ClipboardCache` (`common/Clipboard.hpp:32`), keyed by rotating
  per-session tokens (`wsd/ClientSession.cpp:233`).

### 3. Between two web servers (different coolwsd)

- Full fidelity, with an extra hop. The meta origin points at the source
  server. The browser fetches the rich content from the source endpoint,
  then posts it to the destination endpoint, then issues `.uno:Paste`.
  See `_dataTransferDownloadAndPasteAsync` (`Clipboard.js:341`) and the
  server SET path that re-downloads own-format content from a
  `/cool/clipboard` URL carrying the `X-COOL-Clipboard: true` header
  (`wsd/ClientSession.cpp:446`).

### 4. Native app to native app (same operating system)

- Full fidelity. Both ends use the operating system clipboard with the
  complete native flavor set (Star Embed Source XML, Object Descriptor,
  RTF, HTML, PNG). Copy writes every flavor the Kit reports. Paste reads
  them back and calls `setClipboard`.
- Android adds an on-disk cache file plus instance id so it can restore
  full fidelity across two app instances even though the system clipboard
  only carried HTML (`LOActivity.java:1484,1543`).

### 5. Pasting in from another application

- On paste, each platform reads the clipboard and forwards the flavors it
  recognises to the Kit. Web posts the browser `dataTransfer` contents to
  the endpoint
  (`dataTransferToDocumentFallback`, `Clipboard.js:508`). Each native app
  maps the operating system flavors with its own table, and the tables do
  not agree. The coverage table below is the heart of why one source app
  pastes richly on one platform and degrades on another.

Format coverage by platform (both directions, since scenario 6 uses the
same tables):

| Platform | Paste in: flavors accepted from the OS clipboard | Copy out: flavors published to the OS clipboard |
|---|---|---|
| Web and WASM | Whatever the browser exposes in `dataTransfer`: `text/html`, `text/plain`, image files. No custom MIME types. | `text/html` (with meta-origin), `text/plain`, and `image/png` for image selections. Browser ceiling. |
| macOS | The engine drives the paste through the provider: it lists the pasteboard flavors, each system type mapped back to its MIME type, rich types kept, and pulls only the single format its paste path chooses. | Advertises every flavor the Kit reports, raw, including the internal Star formats under their raw MIME names; bytes are fetched per format on demand by the owner. |
| Windows | Fixed list (`CODA.cpp:771`): plain text, `text/html` (CF_HTML fragment extracted), `text/rtf`, `image/png`, Star Embed Source XML, Star Object Descriptor XML. `image/svg+xml` is recognised but not yet consumed. Anything else is dropped. | The same fixed list (`CODA.cpp:631`): plain text, RTF, HTML, PNG (written under both `image/png` and `PNG`), Star Embed Source XML, Star Object Descriptor XML. |
| Qt | Explicit whitelist (`QtClipboard.cpp:162`): `text/*`, `image/png`, `image/jpeg`, `image/bmp`, `image/svg+*`, `application/x-openoffice-*`, `application/x-libreoffice-*`, ODF `application/vnd.oasis.opendocument.*`, `application/vnd.sun.xml.*`, `application/msword`, `application/mathml+xml`, `application/pdf`. Broadest of the native apps. | Whatever the Kit advertises. The lazy clipboard publishes the full reported flavor list and fetches bytes on demand (`QtClipboard.cpp:92,225`). No whitelist on this side. |
| iOS | Every pasteboard type that maps to a MIME type, forwarded as is. Types with no MIME mapping are skipped (`DocumentViewController.mm:384`). | Every flavor the Kit reports. Plain text and images get a typed representation, and every flavor is also stored raw under its MIME key (`DocumentViewController.mm:307`). |
| Android | One flavor only, first match in priority order HTML, then `image/*`, then plain text (`LOActivity.java:1535,1568,1593`). | `text/html` plus plain text only, through `ClipData.newHtmlText` (`LOActivity.java:1515`). Full fidelity is kept in a side cache file, not on the system clipboard. |

Takeaways:
- macOS and iOS are the most permissive in both directions. They forward
  whatever the operating system clipboard carries and publish the full
  native set.
- Qt accepts a broad explicit set that uniquely includes ODF, PDF, and the
  legacy binary DOC word-processing format, and advertises everything on
  copy out.
- Windows is limited to a hand-maintained list. Rich text survives as
  HTML, RTF, PNG, and the two internal Star formats. Anything outside the
  list is silently dropped, both ways.
- Android is the weakest for cross-app exchange. Paste keeps only a single
  best format, and copy out exposes only HTML and plain text to other
  apps.
- Web and WASM are bounded by the browser. HTML, plain text, and PNG only,
  with no custom MIME types in either direction.

### 6. Copying out to another application

- The other app gets the richest flavor the platform published from the
  copy-out column above. The reach is widest on macOS, iOS, and Qt, which
  put the full native set on the clipboard, so a strong target app can
  take ODF, RTF, or the internal Star embed formats.
- Windows offers HTML, RTF, PNG, and the two Star formats, so common
  targets get formatted text but exotic formats are absent.
- Android and the web and WASM builds expose only HTML, plain text, and
  (where relevant) PNG, so the external app cannot do better than HTML
  fidelity.

### 7. Browser COOL to a native app, and the reverse

This is the cross-environment case and today it is the weakest link. Both
directions go through the operating system clipboard, which cannot carry
what each side actually needs.

- **Browser to native app.** The browser only placed HTML (with the
  meta-origin div) plus plain text on the operating system clipboard. The
  rich ODF and embed-source bytes are still on the source web server. The
  native app reads the clipboard, sees HTML, and pastes at HTML fidelity.
  It does not follow the meta-origin URL to fetch the server content, so
  the embed-source fidelity is lost.
- **Native app to browser.** The native app published the full flavor set
  on the operating system clipboard, but the browser cannot see custom
  MIME types like `application/x-openoffice-embed-source-xml` from
  JavaScript. The browser paste gets HTML at best.
- Net effect: browser and native interoperate at roughly HTML fidelity,
  not at the native or server fidelity each side is capable of on its own.

### 8. Images and Paste Special

- Images paste as `image/png` through a typed blob message on web
  (`_pasteTypedBlob`, `Clipboard.js:417`) and as PNG flavors on the
  native apps. Small image selections can ride inline on the clipboard.
- Paste Special chooses `.uno:PasteSpecial` over `.uno:Paste`
  (`_checkAndDisablePasteSpecial`, `Clipboard.js:1635`). The Kit honours
  the explicit MIME type and skips format sniffing
  (`init.cxx:6538`).

### 9. Calc cut and move, and large selections

- A Calc cut deletes the source at once and records an undo step
  (`CutToClip`, `viewfun3.cxx:193`). There are no marching ants, the
  source overlay is switched off whenever the Kit is active, which covers
  every build here (`gridwin.cxx:6480`).
- Cut as move, where formula references are adjusted as a move rather than
  a copy, only happens on a same-document paste. The Kit keeps the live cut
  transferable with its cut flag (`ScClipParam::mbCutMode`), and the
  reference update runs in move mode only when the source and destination
  are the same document (`document.cxx:2838`). The operating system
  clipboard carries no cut bit, so a paste from any other origin (another
  document, server, or app) becomes a plain copy. Non-contiguous
  multi-range selections never carry the cut flag at all
  (`viewfun3.cxx:469`).
- A large selection (a whole sheet, or roughly more than a thousand cells,
  or ten thousand characters) is flagged complex (`transobj.cxx:764`,
  `init.cxx:6369`). The Kit sends `complexselection:` instead of inline
  content, and the client downloads the bytes from the clipboard endpoint
  on demand (`Clipboard.js:1420`, async GET), so a whole-sheet copy does
  not block on an inline transfer.
- Moving a whole sheet by its tab ("Move or Copy Sheet") does not use the
  clipboard at all. It is a direct core operation (`FID_TAB_MOVE`,
  `tabvwshf.cxx:207`), and a sheet-tab drag is desktop drag and drop, not
  the operating system clipboard.
- Calc offers a rich flavor set on copy: the Star embed-source and object
  descriptor, a metafile, PNG and bitmap, HTML, SYLK, a DDE link, DIF,
  plain text and tab-separated text, RTF, and Markdown
  (`transobj.cxx:206`). There is no binary spreadsheet interchange format.
  A very large bitmap is clamped to avoid an allocation crash
  (`transobj.cxx:593`, tdf#160855).

## Non-document content (formula bar, comments, inputs)

Not everything the user copies lives in the document canvas. The formula
bar, comment edit boxes, the find box, the name box, and jsdialog text
fields are browser widgets, and clipboard there mostly bypasses the Kit.

- The document clipboard path bails out when focus is in one of these
  widgets. `_isAnyInputFieldSelected` (`Clipboard.js:628`) returns true for
  the find box, jsdialog inputs and text areas, open dialogs, and comment
  edit boxes (`cool.Comment.isAnyFocus`), and for copy also when there is
  any non-empty window selection. Copy, cut, and paste then fall through to
  the browser's own input handling, and the `.uno:` round trip never fires.
- Comments and replies are contenteditable boxes with no clipboard
  handlers (`CommentSection.ts:261`), so their copy and paste are purely
  browser-native. The text is harvested on commit through innerText and
  innerHTML, with sanitisation.
- The formula bar is the one hybrid, and it is asymmetric. Paste into it is
  routed to the document path on purpose (`_isFormulabarSelected`,
  `Clipboard.js:659`, negated in `paste()`), so external content reaches
  the cell edit through `.uno:Paste`. Copy and cut from it have no such
  exception and stay browser-native. Copying a cell that holds a formula
  keeps the raw `=...` off the client clipboard on purpose and exports the
  rendered form instead (`setTextSelectionText`, `Clipboard.js:1402`).
- Two fragilities worth noting. The formula-bar paste check keys on
  `#sc_input_window` being `:focus`, but interaction moves focus to a
  shared hidden text area, so the check can read false at paste time
  (`Widget.FormulabarEdit.js:446`). And the Calc name box (`addressInput`)
  is a plain input with no dedicated short-circuit, so its clipboard is
  browser-native by default rather than by design.

## What is shared

- The Kit API and the headless `KitClipboard` are shared by every
  platform.
- The serialization format in `common/ClipboardData.hpp` is shared by the
  server, Windows, Qt, iOS, and Android when they read or write the tuple
  blob.
- `browser/src/map/Clipboard.js` is the one client module for all
  platforms.
- The two-pass clipboard capture (ask for all flavors, then ask
  explicitly for `text/plain;charset=utf-8` and `text/html`) is the same
  idea on iOS and Android, and iOS is a hand-port of the Android code
  (comment at `DocumentViewController.mm:306`).

## What is not shared (duplicated per platform)

This section describes the state before the COKitClipboardProvider move and
still holds for Windows, Qt, iOS, and Android. macOS has since moved to the
provider (see the Done section): the engine now decides the formats, so on
macOS the mapping table is reached through the provider callbacks and the
old `write` and `setClipboardWith` paths named below are gone. The
duplication described here is what the provider move is meant to remove,
one app at a time.

- **Operating system to MIME mapping tables.** Written five times, each
  covering a slightly different format set:
  `COWrapper.mm:187`, `CODA.cpp:631`, `QtClipboard.cpp:162`,
  `DocumentViewController.mm` (UTI mapping), `androidapp.cpp:355`. There
  is no common translation table, so a format supported on one platform
  can silently drop on another.
  - Stays partly separate: the native half of each lookup speaks a
    different type system. macOS and iOS use UTType, Windows uses numeric
    clipboard format ids, Qt and Android already speak MIME strings. The
    lookup calls cannot collapse into one function.
  - Could be shared: the MIME half could be one declarative list. Which
    engine MIME types matter, the `text/plain;charset=utf-8`
    canonicalisation, and the Star Embed Source and Object Descriptor
    names are the same everywhere. macOS and iOS use the same UTType API
    and could share the code outright, and iOS is already a hand-copy of
    Android. Windows is the only platform that truly needs a format-id
    translation step.
- **The own-clipboard optimisation.** Implemented independently with five
  different mechanisms: NSPasteboard changeCount, Win32 sequence number,
  Qt `ownsClipboard` plus source document id, Android magic plus instance
  id plus cache file, and the web meta-origin token match. iOS leans on
  the `sendToInternal` direct path instead.
  - Stays separate: the primitive question "has the clipboard changed
    since I last wrote it" has no cross-platform answer. Each native check
    is the only one its operating system offers.
  - Could be shared: the decision built on top is identical everywhere. If
    we still own the clipboard, issue `.uno:Paste`, otherwise read then
    set then paste. That decision already lives once in the browser JS
    (`dataTransferToDocument`, `Clipboard.js:466`), and each native app
    re-derives it. A shared helper that takes a single per-platform "do I
    still own it" boolean would remove the duplication. iOS skips the
    optimisation entirely and could adopt the same model.
- **The own-content identity marker.** The web and the desktop and iOS
  apps use the `data-coolorigin` meta-origin div. Android invents its own
  `cool-clip-magic-<instance id>` signature. Two schemes for one concept.
  - Nothing needs this separate. The `data-coolorigin` div is already
    produced and stripped in shared code (browser JS and
    `wsd/ClientSession.cpp`). Android could read and write the same
    marker. Its extra per-instance id can be appended to that marker or
    taken from the `ViewId` and `Tag` already carried in the URL. This is
    pure technical debt.
- **The native message protocol.** macOS and iOS use a dedicated
  `clipboard` script handler with read, write, sendToInternal. Windows
  and Qt use the generic `postMobileMessage` channel with COPY, CUT,
  PASTE, CLIPBOARDREAD, CLIPBOARDWRITE, CLIPBOARDSET, CLIPBOARDMIMETYPES.
  Android uses plain `uno` commands plus JNI side effects. Only the
  `TEXTCLIPBOARD` plain-text fast path is common across the desktop trio.
  - This is not only different verb names. It is three different divisions
    of work between JavaScript and native code:
    - macOS and iOS: the `clipboard` handler is a pure byte mover (read,
      write, sendToInternal). The `.uno:Copy` and `.uno:Paste` command
      goes separately over the fake socket.
    - Windows and Qt: the COPY, CUT, and PASTE verbs bundle the UNO
      command and the clipboard move into one message, so the native side
      both runs the command and touches the operating system clipboard.
    - Android: sends no clipboard verbs at all. It sends the plain `uno`
      command and does the clipboard work as a side effect of seeing that
      command.
  - All native models already move the bytes in process. macOS and iOS
    read the pasteboard and call the Kit `setClipboard` straight from
    native code (`COWrapper.mm:298`), and `sendToInternal` deliberately
    discards the HTML that JavaScript handed it and reads the pasteboard
    itself (`COWrapper.mm:366`). Windows and Qt do the same. So neither
    model pays a base64 round trip for an ordinary copy or paste. The
    string-bridge hop and the read-reply-back-to-JavaScript show up in
    exactly one place, the `navigator.clipboard` read emulation that macOS
    and iOS feed through the `read` verb (`Clipboard.js:1117`). The Windows
    half of that emulation is a dead path, with a code comment that the
    reply "is surely wrong" and that WebView2 cannot return a value anyway
    (`Clipboard.js:1122`). Windows still pastes correctly, because
    `do_paste_or_read` does the whole paste in native code and returns
    nothing to JavaScript.
  - Recommendation: take the Windows model as the base, not the macOS and
    iOS one. JavaScript sends one high-level intent (COPY, CUT,
    PASTE, PASTESPECIAL) and native does the entire operation. It runs the
    UNO command on the in-process Kit and moves the bytes straight between
    the Kit and the operating system clipboard. JavaScript never sees the
    bytes and never waits for an answer.
  - Why this is the better base to share:
    - Transport stops mattering. One-way fire-and-forget works on every
      embedded browser, so there is no inline reply to engineer: no
      WebView2 host object, no correlation id, no QWebChannel return value.
      The read-reply obstacle and the dead `_WindowsReadClipboard` path are
      both gone.
    - Highest fidelity and lowest overhead: the bytes stay in process and
      never pass through a string bridge.
    - The own-clipboard check is native anyway (changeCount, sequence
      number, `ownsClipboard`), so it lives where it acts instead of being
      mirrored into JavaScript.
    - The shared part collapses to one place, since the sequence is the
      same everywhere. Copy is getClipboard, write to the operating system
      clipboard, post Copy. Paste is: still ours, post Paste, otherwise
      read the operating system clipboard, setClipboard, post Paste. That
      is one C++ helper for macOS, iOS, Windows, Qt, and Android, behind a
      thin per-platform adapter with three calls: write flavors, read
      flavors, and has-it-changed. The five mapping tables become those
      adapter bodies, the one part that must stay per platform.
  - What stays in JavaScript: picking the verb. The shared `Clipboard.js`
    still decides Copy versus Cut and Paste versus Paste Special, and
    catches the keyboard and menu events. It sends the verb and stops, the
    same fire-and-forget call everywhere.
  - Honest disadvantages and scope:
    - Less logic lives in the shared JavaScript module and moves to a
      shared C++ helper instead. Fair trade for the five in-process-kit
      apps, which already do the real transfer in native code.
    - It does nothing for the web server build or WASM. Their Kit is remote
      or sandboxed, so content must travel through HTTP or
      `navigator.clipboard` regardless. Those two keep the JavaScript path
      (the split at the top of this document). The shared C++ bridge is for
      in-process-kit apps only.
    - Android is the most work: it has no clipboard verbs today and drives
      the clipboard as a side effect of `uno` commands. It would gain the
      verbs and call the shared helper through JNI.
    - Gesture-sensitive, well-tested code (see the Safari await caveat
      below), so consolidation should sit behind current behaviour and be
      cut over carefully.

## Exceptions and quirks worth knowing

- **DisableCopy (WOPI host policy).** The server refuses clipboard GET
  with Forbidden, returns an empty clipboard URL, and does not cache
  content (`wsd/ClientSession.cpp:249,355,3031`). The browser puts a
  "copying disabled" stub on the clipboard
  (`_getDisabledCopyStubHtml`, `Clipboard.js:205`).
- **Async clipboard permission timing.** On Safari and mobile the code
  must not `await` before `navigator.clipboard.write`, or the user
  gesture permission is dropped. Heavily commented at `Clipboard.js:954`.
  Failures while the document is unfocused are retried on the next focus
  event (`Clipboard.js:1011`).
- **Binary over string bridges.** The mobile native bridges carry strings
  only, so binary paste payloads are base64 encoded
  (`_pasteTypedBlob`, `Clipboard.js:420`) and decoded under
  `#if MOBILEAPP` in `kit/ChildSession.cpp:1836`.
- **Token lifetime.** Per-session clipboard tokens are 16 random bytes,
  two are kept (current and previous), they rotate every 5 minutes
  (`wsd/DocumentBroker.cpp:729`), and the disconnected-view cache expires
  after 10 minutes (`common/Common.hpp:42`).
- **Windows CF_HTML.** Windows reads back the CF_HTML fragment with a
  regular expression over the `StartFragment` and `EndFragment` markers
  (`CODA.cpp:709`).
- **Qt lazy clipboard.** Qt advertises the flavor list up front and only
  fetches bytes when a target asks, then forces a full fetch on document
  close so an external paste survives (`QtClipboard.cpp:92,232`).
- **iOS window paste.** The dialog paste branch in the Kit is compiled
  out on iOS (`#ifndef IOS`, `init.cxx:7662`).
- **windows/clipboard.cpp is not the app bridge.** It is a standalone
  command-line clipboard utility. The real Windows bridge is
  `windows/coda/CODA/CODA.cpp`.

## Findings on the cross-environment questions

Evidence confirming the open points in scenario 7 and the sharing notes
above.

- Browser to native paste is confirmed HTML-only on every platform. No
  native app reads the meta-origin URL or makes any network request on
  paste. Its clipboard "fetch" calls all pull from the in-process Kit
  (`COWrapper.mm:204`, `QtClipboard.cpp:50`), and Android's cross-instance
  restore reads a local cache file
  (`getCacheDir()/LibreofficeClipboardFile.data`,
  `LOActivity.java:1486,1546`), not the source server.
- For the native to browser direction there is no server URL to hand the
  browser, and the only web-platform fit is web custom clipboard formats
  (the `web ` type prefix on the async Clipboard API). The project uses
  them nowhere yet, and they are Chromium-only and need the native side to
  emit the exact operating system representation the browser expects.
- Unifying the five MIME tables is safe as long as each per-platform
  adapter still gates on what its clipboard API can carry. The parts that
  must stay per platform are the Windows CF_HTML fragment extraction
  (`CODA.cpp:709`), the plain-text normalisation each platform already
  does, and the `windows_formatname` parameter on the Star embed-source
  MIME that the Kit paste path rewrites (`kit/ChildSession.cpp:1827`).

## Requirements check

This design measured against an external requirements list.

| Requirement | Status | Notes |
|---|---|---|
| PC to PC copy and paste, including Calc cut and move of large sheets | Mostly met | Native desktop copy carries the full flavor set (scenario 4), and large or whole-sheet selections download on demand (scenario 9). Gap: cut as move is same-document only, so a cut pasted into another instance becomes a copy. |
| Works for non-document content (comments, formula-bar text, core text) | Works, but by bypass | Comments and inputs are browser-native, the formula bar is a hybrid with a copy-versus-paste asymmetry and a fragile focus check (see Non-document content). No single model spans widgets and document text. |
| Better clipboard for collaboration inside CODA | Partial | Copy and paste between documents on one server keep full fidelity through the server-held transferable (scenarios 2 and 3). More comes from the reworks below. |
| Efficient remote download when pasting remote into local | Not built | The path exists for the browser cross-server case and would be reused by the native fetch (action plan, browser to native rich paste). |
| Super-bonus: hook and push PC to online pastes in the CODA shell | Feasible, natural home | The shell has native operating system clipboard access and the in-process Kit, so it can read the system clipboard and push it into the document. It rides the shared C++ bridge (action plan). |
| Shares lots of code cross-platform | Goal of the rework | The shared C++ bridge replaces five native glue layers with one helper plus thin adapters (action plan). |

## Done

### macOS: engine-driven clipboard through COKitClipboardProvider

The macOS app no longer copies bytes between NSPasteboard and the Kit with
its own per-flavor logic. The engine decides which formats matter, and the
app does only raw NSPasteboard input and output, through a provider the app
registers per document. This is the engine-driven model: it differs from
the "Windows native does everything" base proposed in the native message
protocol notes above, because the engine, not the app, picks the useful
formats. The app shrinks to a thin raw input and output adapter, so the
per-app "which flavors matter" guesswork is gone.

- New Kit API. `COKitClipboardProvider` and the `installClipboardProvider`
  document entry point (`engine/include/COKit/COKitTypes.h`,
  `engine/include/COKit/COKit.h`). The kit clipboard uses the provider in
  both directions (`engine/desktop/source/lib/kitclipboard.cxx`). On copy
  `KitClipboard::setContents` advertises the engine's own flavor list
  straight onto the pasteboard through the provider, in process, instead of
  the `clipboardmimetypes:` notification. On an external paste a new
  `KitProviderTransferable` reads the pasteboard one format at a time, so
  the engine's existing format-priority choice governs and only the chosen
  format is fetched. The own-clipboard shortcut, where the paste comes from
  the in-memory transferable when we still own the clipboard, is preserved
  through the provider `ownsClipboard` callback. Installing a provider is
  refused unless the document work runs on the app's own thread, so the
  out-of-process server keeps the plain `getClipboard` and `setClipboard`
  path untouched.
- macOS side (`macos/coda/coda/COWrapper.mm`). Implements the provider:
  advertise through a retained `COClipboardOwner` that serves a format's
  bytes on demand, list and read the pasteboard for paste, and the
  `ownsClipboard` check on the pasteboard change count. The old eager paths
  are removed: the `write` script message, the whole-clipboard
  `putOnPasteboard` write, and the read-everything `setClipboardWith`.
- No browser round-trip for advertise. Because the engine calls the
  provider in process, the `clipboardmimetypes:` message and its forwarding
  (`browser/src/layer/tile/CanvasTileLayer.js`, the `ViewController`
  handler) are gone for macOS. That message path stays only for the
  server, the web and WASM clients, and the Qt app, which are not yet on
  the provider.

## Action plan

Ordered by effort. Each item points back to the section above that
explains it.

### Low-hanging fruit

- Delete the dead Windows read path. `_WindowsReadClipboard`
  (`Clipboard.js:1122`) awaits a value that `postMobileMessage` cannot
  return, and a code comment already says so. Windows paste does not use
  it. Remove it and its branch in `_asyncAttemptNavigatorClipboardRead`
  (`Clipboard.js:1142`).
- Collapse the redundant two-pass clipboard capture on iOS and Android.
  Both call `getClipboard` once for all flavors and again only for
  `text/plain;charset=utf-8` and `text/html`
  (`DocumentViewController.mm:307`, `androidapp.cpp:355`). The first pass
  already returns those, so the second is wasted work.
- Extract one canonical MIME list. Pull the supported MIME types, the
  plain-text normalisation, and the Star format names into a single shared
  header that the five mapping tables read from, even before the adapters
  are refactored. This makes the supported set auditable in one place and
  is the first step of the larger rework.
- Align the own-content marker. Have Android read and write the shared
  `data-coolorigin` div instead of its own `cool-clip-magic` signature,
  and take the per-instance id from the `ViewId` and `Tag` already in the
  URL.
- Harden the formula-bar clipboard path. Paste keys on `#sc_input_window`
  being `:focus` while focus actually sits on a hidden text area
  (`Widget.FormulabarEdit.js:446`), so the check can read false. Copy and
  cut from the formula bar stay browser-native while paste is routed to
  core, an asymmetry. Make the focus test robust and settle on one model
  for both directions. The Calc name box has the same browser-native gap.

### Larger reworks

- Move Qt and Windows onto the COKitClipboardProvider (the model macOS now
  uses, see the Done section). Each app registers a provider per document
  and implements the same small surface: advertise a flavor list on copy,
  list the pasteboard flavors, fetch one format's bytes, and answer the
  own-clipboard check. The engine then drives both directions and picks the
  paste format, so the per-platform "which flavors matter" lists (the Qt
  whitelist, the Windows fixed list) and the eager copy out go away. Qt
  already has a lazy clipboard, so it is the smaller step; Windows still
  serializes every format up front and gains lazy copy from the move. Start
  with Qt, then Windows; iOS and Android can follow later. This is the
  chosen shape of the "shares lots of code" goal for the in-process apps:
  the shared logic lives in the engine instead of a shared C++ helper, and
  each app is left a thin raw input and output adapter.
- Browser to native rich paste (scenario 7, the easy half). Teach the
  native apps to spot a `data-coolorigin` URL in pasted HTML, fetch the
  rich content from the source server in one asynchronous HTTP request, and
  feed it to the local Kit through `setClipboard`. This reuses the existing
  `/cool/clipboard` endpoint, the same path the browser already uses for
  cross-server paste (`Clipboard.js:341`), and lifts that paste from HTML
  up to server fidelity. Start with one platform.
- One shared C++ clipboard bridge (the native message protocol rework).
  Superseded for the in-process apps by the COKitClipboardProvider move
  above: that keeps the shared logic in the engine rather than a shared C++
  helper, and reaches the same "native does the transfer, JavaScript only
  picks the verb" end state. The reasoning is kept here because it still
  describes the goal and the trade-offs. Move the copy and paste sequence
  into a single C++ helper linked by macOS, iOS, Windows, Qt, and Android,
  behind a thin per-platform adapter with three calls: write flavors, read
  flavors, and has-it-changed. Adopt the Windows native-does-everything
  model, so JavaScript sends one fire-and-forget verb and never needs a
  reply. This folds in the MIME table unification, the own-clipboard check,
  and a single verb vocabulary. Android needs the most work, since it has no
  clipboard verbs today. This is also the home for the super-bonus, hooking
  the system clipboard in the CODA shell and pushing it into the online
  document, since the native side already reads the operating system
  clipboard.
- Native to browser rich paste (scenario 7, the hard half). Decide whether
  to invest in the web custom formats path described in the findings, or to
  accept HTML fidelity in this direction. Lowest priority of the three,
  because the payoff is narrow and the fragility is high.
