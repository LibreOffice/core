# Dynamic zoom in Collabora Online

This document explains "dynamic zoom" (also called "smart zoom") on the
Writer/Impress document load path: what it is, how it works, and the bugs and gotchas
you are likely to hit. It is written for someone new to the zoom/scaling area who
needs to fix or extend something here, or just to understand it.

---

## 1. The problem

When a document loads, the client picks an initial zoom level so the page fits the
available width (fit-width). Getting that zoom right is on the hot path for startup:
changing the zoom means re-requesting and re-rendering tiles, and tile invalidation
is expensive, especially the bulk invalidation that a zoom change triggers.

A purely reactive approach makes this worse: set a default zoom (100%), start
requesting tiles, then recalculate the zoom whenever an event arrives that changes
the available space -

- the "we have comments" flag (partHasComments) coming from the engine,
- the sidebar showing up (it shrinks the canvas),
- browser dpi-scale / resize events.

Each of these arrives asynchronously, often one after another, and each resets the
zoom while tiles are already rendering. A typical bad sequence: the document starts at
100%, smart zoom kicks in and jumps to 170% (refetch tiles), the comments flag arrives
(refetch), the sidebar appears (refetch). The tiles get invalidated several times
before settling, which is slow and wasteful.

The approach: compute the correct zoom once, as early as possible, before the first
tile request, using all the information that affects it - so there is no mid-load zoom
change and no repeated tile invalidation.

---

## 2. Units and the zoom scale (read this first)

Almost every bug in this area is a units bug. There are three coordinate units, plus
a logarithmic zoom scale.

### Three units

- **twips** - the base unit. A twip is one twentieth of a point, and there are 72
  points per inch, so 1 twip = 1/1440 inch. The engine reports document sizes in
  twips. CSS treats the screen as 96 dpi, and 1440/96 = 15, so **1 CSS pixel = 15
  twips**.
- **CSS pixels** - the number of pixels an element would take on a 96 dpi display.
  This is what you position HTML elements with.
- **canvas / core pixels** - the actual number of device pixels used to draw the
  element. On a high-density or browser-zoomed display this is larger than the CSS
  pixel count.

`window.devicePixelRatio` is how many device pixels are used to draw one CSS pixel.
In this codebase that number is `app.dpiScale`. It is a **divider**: to go from core
pixels to CSS pixels you divide by `app.dpiScale`; to go from CSS pixels to core
pixels you multiply. `app.dpiScale` changes when the user changes browser zoom or
display scaling.

The long comment at the top of `browser/src/core/geometry.ts` is the canonical
explanation and is worth reading.

### The zoom scale

The map's zoom is a small integer on a logarithmic scale, not a percentage.

- The default is `10`, which means 100% (ratio 1.0).
- Each step away from 10 multiplies or divides by pow(2, 1/4), roughly 1.19.
- So zoom 4 is six divisions (about 35%), zoom 18 is eight multiplications (about
  400%). The available percentages are rounded to the nearest 5%.

`Map.js` owns this. Two functions convert between a scale ratio and the integer:

- `getZoomScale(toZoom, fromZoom)` -> a ratio,
- `getScaleZoom(ratio, fromZoom)` -> a zoom integer.

The dynamic-zoom helpers compute a width ratio (available width / document width) and
call `getScaleZoom(ratio)` to turn it into a zoom integer.

### Tiles

A tile is a fixed-size square bitmap that is stitched together with others to form
the document. It is 256 CSS pixels, which is 256 * 15 = 3840 twips, at the default
zoom. `window.tileSize = 256` (in `global.js`); `tileWidthTwips = tileSize * 15` (in
`Map.js`). The map's `defaultZoom: 10` is defined as the zoom at which a tile is
exactly this default twip size.

---

## 3. The startup message flow

Two engine messages matter for initial zoom, and they arrive in this order:

1. **`browsersetting:`** - arrives well before the document loads. It carries the
   per-doctype UI preferences and the zoom prefs. The ones that matter here:
   `smartZoom` (true/false), `defaultZoom` (the fallback zoom when smart zoom is
   off), and `text.ShowSidebar` (whether the sidebar will be shown). It is handled in
   `Socket.ts` `_onMessage`, which calls `window.prefs._initializeBrowserSetting`;
   after that the values are readable via `window.prefs.getBoolean(...)` /
   `window.prefs.get(...)`.

2. **`status:`** - the first one carries `width`, `height`, `pagerectangles`,
   `partscount`, `selectedpart`, `partHasComments`, `type`, `readonly`. This message
   is what triggers the canvas layer to be added.

The status message is prepared on the engine side in `kit/KitHelper.hpp`,
`LOKitHelper::documentStatus`. `width`/`height` come from `getDocumentSize` (twips),
and `partHasComments` comes from doctype-specific code (for Writer,
`fetchWriterSpecificData`).

### The onAdd path

The first status message creates the tile layer and drives this chain:

```
Socket._onStatusMsg  ->  addLayer  ->  Layer._layerAdd  ->  CanvasTileLayer.onAdd
    ->  _initContainer  ->  _syncTileContainerSize  ->  _fitWidthZoom  (call #1)
```

`_fitWidthZoom` call #1 does nothing useful, because at this point
`app.activeDocument.fileSize.x` is still 0 (it returns early).

`Socket._onStatusMsg` also puts the document width and `partHasComments` from the
status message into the layer options as `documentWidthTwips` and `partHasComments`,
so by the time `onAdd` runs they are on the layer as `this._documentWidthTwips` and
`this._partHasComments` (copied out of the options in `initialize`). Two things then
happen inside `onAdd`:

1. **Show the sidebar early.** If `text.ShowSidebar` is set, `onAdd` calls
   `map.sidebar.showSidebar()` straight away - an empty sidebar, before its contents
   arrive. This shrinks the document container now, so the container size read a
   moment later already accounts for it.

2. **Compute and set the zoom early.** `onAdd` calls `setInitialZoom`, and for Writer
   the fit-width zoom is computed from the values forwarded through the map options -
   `this._documentWidthTwips` for the document width and `this._partHasComments` to
   decide whether to reserve the comment column - together with the current container
   size, and applied with `map.setZoom(zoom, {animate: false})`. The calculation itself
   is in section 4.

`_sendClientZoom` is the call that tells the engine which zoom to render tiles for,
and it runs later, after `onAdd` has finished. So everything above - showing the
sidebar, reading the container size, computing and setting the zoom - is already done
before the zoom is sent. When it is finally sent it is the correct one, so there is no
later zoom change and no re-fetch. The 100% default (`Map.initialize` sets `zoom: 10`)
is overwritten by the `setZoom` in `onAdd` before `_sendClientZoom` ever reads it.

---

## 4. Computing the zoom early

In `CanvasTileLayer.onAdd`, before requesting any tiles, everything that affects the
fit-width zoom is gathered and the zoom is set once.

What is needed to compute it early:

- **document width** - taken from the `width` field of the first status message and
  passed into the layer (previously only available later, once tiles arrived),
- **whether there are comments** - the `partHasComments` flag from the status
  message, also passed into the layer,
- **whether the sidebar will show** - the `text.ShowSidebar` browser setting, already
  available by this point.

The sidebar is shown *early and empty* in `onAdd` when the setting says it will be
shown. Showing it shrinks the document container, so the container size read
immediately afterwards already accounts for it, and the zoom computed from that
container size does not need to be revised when the real sidebar contents arrive.

`onAdd` calls `setInitialZoom`, which holds the whole sequence (Writer path):

```js
setInitialZoom: function (map) {
    if (this.isWriter()) {
        const smartZoomEnabled = window.prefs.get('smartZoom') != 'false';
        const maxZoom = this._getMaxZoom();
        let zoom;
        if (smartZoomEnabled) {
            const documentWidth = (this._documentWidthTwips / app.tile.size.x) * (RenderManager.tileSize * app.dpiScale);
            const containerSize = this._getDocumentContainerSize()[0] * app.dpiScale;
            zoom = this._writerDynamicZoom(containerSize, documentWidth, this._partHasComments);
        } else {
            zoom = this._getWriterDefaultZoom();
        }
        if (maxZoom) zoom = Math.min(maxZoom, Math.max(0.1, zoom));
        if (zoom > 1) zoom = Math.floor(zoom);
        this._firstFitDone = true;
        if (this._partHasComments) this._includedCommentsInFirstFit = true;
        map.setZoom(zoom, {animate: false});
    } else {
        map.setZoom();
    }
    this._viewReset();
    this._resetClientVisArea();
    this._requestNewTiles();
}
```

Those last three calls run *after* `setZoom`. Setting the zoom first means the tile
request that follows already carries the final zoom, so the tiles are fetched once and
at the size they will be drawn at. Keeping the zoom and the three calls in one method
is what fixes that order.

`this._firstFitDone = true` marks that the initial fit has been done, so the reactive
`_fitWidthZoom` path will not redo it when later resize events fire.

### The zoom helpers

The dynamic-zoom math lives in two doctype helpers, shared by both the early `onAdd`
path and the reactive `_fitWidthZoom` path:

- **`_writerDynamicZoom(containerWidth, documentWidth, bringCommentsIntoView)`** -
  subtracts the comment column width from the container when comments are in view,
  then returns `getScaleZoom(containerWidth / documentWidth)`.

- **`_impressDynamicZoom(containerWidth, containerHeight, documentWidth, documentHeight)`** -
  computes a window-size factor from the screen diagonal, picks a margin between 4%
  and 9% based on it, and fits by the smaller of the width and height ratios.

`_recalcZoom(newSize, bringCommentsIntoView, maxZoom)` now just dispatches to one of
these based on doctype and clamps to `maxZoom`. Calc, Draw and multi-page views do
not get dynamic zoom.

---

## 5. Key files and functions

- **`browser/src/app/Socket.ts`** - `_onStatusMsg` builds the doc-layer options from
  the status command and creates the doctype tile layer. This is where the status
  `width` becomes the `documentWidthTwips` option and `partHasComments` is put into
  the options that reach the layer.
- **`browser/src/layer/tile/CanvasTileLayer.js`** - the heart of it: `onAdd`,
  `setInitialZoom`,
  `_writerDynamicZoom`, `_impressDynamicZoom`, `_recalcZoom`, `_fitWidthZoom`,
  `_sendClientZoom`, `_getWriterDefaultZoom`, `_getMaxZoom`,
  `_getDocumentContainerSize`.
- **`browser/src/map/Map.js`** - zoom state and the scale/zoom conversions
  (`getZoomScale`, `getScaleZoom`, `setZoom`, `getZoomPercent`), and `panBy` for
  scrolling (scrolling moves the map pane).
- **`browser/src/app/ViewLayoutWriter.ts`** - Writer comment/margin handling:
  `getCommentAndDocumentSpacingInfo`, `adjustDocumentMarginsForComments`, and the
  document-move-left logic that slides the page over to make room for comments.
- **`browser/src/canvas/sections/CommentListSection.ts`** - `calculateAvailableSpace`
  and `commentWidth`.
- **`browser/src/core/geometry.ts`** - the units explainer comment.
- **Engine side**: `kit/KitHelper.hpp` (`documentStatus`), and for Writer
  `engine/sw/source/uibase/uno/unotxdoc.cxx` (`SwXTextDocument::partHasComments`).
