# Online JS extensions

An extension is a directory (or zip, for the integrator preset pipeline) containing a
`manifest.json` plus whatever HTML/JS/CSS/icon files it references.

## manifest.json fields

| Field | Meaning |
|---|---|
| `manifestVersion` | Currently always `"0.1"`. Bumped on backwards-incompatible format changes. |
| `name` | Human-readable name shown in the notebookbar tab, menu, and panel header when `entry` is set. Unused for a commands-only extension. |
| `entry` | Path (relative to the manifest) to the HTML page loaded into the sidebar panel's iframe. Omit it for a commands-only extension with no sidebar UI. |
| `icon` | Path (relative to the manifest) to an icon shown wherever the extension is referenced, when `entry` is set. Unused for a commands-only extension - see `contributes.commands[].icon` for a per-command icon instead. |
| `supports` | Array of doc classes: `"text"`, `"spreadsheet"`, `"presentation"`, `"drawing"`. Absent or empty means all. |
| `contributes` | See below. |

An extension with `entry` gets a toggle button in the notebookbar's "Extensions" tab
and the compact-UI "Extensions" menu; activating it opens `entry` as a foldable
sidebar panel. This is unrelated to `contributes` below and works the same regardless
of whether the extension also declares commands.

## contributes

Lets an extension put commands directly into the classic menu and/or the notebookbar
(ribbon), without needing its sidebar panel (if it has one) to ever be opened.
`contributes` is a string naming a separate JSON file (relative to the manifest,
resolved the same way `entry`/`icon` are) holding the object described below -
kept out of `manifest.json` itself so an extension's UI wiring, however long it
gets, never crowds out that file's own metadata.

manifest.json:

```json
{
  "manifestVersion": "0.1",
  "name": "Commands demo",
  "supports": ["text"],
  "contributes": "ui.json"
}
```

ui.json:

```json
{
  "commands": [
    { "id": "insertDate", "title": "Insert Date", "icon": "icon.svg", "script": "commands/dates.js" }
  ],
  "menus": {
    "insert": ["insertDate"]
  },
  "notebookbar": [
    {
      "tab": "Commands Demo",
      "insertAfter": "Home",
      "groups": [
        {
          "id": "dateGroup",
          "label": "Date",
          "items": [
            { "type": "button", "command": "insertDate", "size": "large" }
          ]
        }
      ]
    }
  ]
}
```

- `contributes.commands` - each entry's `script` is a path (relative to the manifest)
  to a JS file whose top-level binding named `commands` is an object mapping
  command ids to functions, e.g.:

  ```js
  var commands = {
      insertDate: function () {
          var doc = cool.getActiveDocument();
          // ... call into the document via the UNO API ...
      },
  };
  ```

  More than one command may name the same `script` file - add another key to its
  `commands` object rather than giving each command a one-function file of its own.
  A referenced function runs when its command is chosen from the menu or
  notebookbar. It only has access to the UNO API shown above - no DOM, no network,
  no other browser capability - and it runs without ever loading a visible panel,
  so the command works whether or not the extension has a sidebar UI at all. `icon`
  is optional and shown on a notebookbar button or dropdown entry that references
  this command - the classic menu never renders it.

- `contributes.menus` maps an existing top-level classic-menu id (`file`, `editmenu`,
  `view`, `insert`, `format`, and so on, depending on the document type) to a list of
  command ids appended to the end of that menu.

- `contributes.notebookbar` is a list of brand-new ribbon tabs the extension adds -
  `{ tab, insertBefore?, insertAfter?, groups }`. `tab` is the new tab's own
  label and must not collide with an existing tab's name (`Home`, `Insert`, `Format`,
  ...). `insertBefore`/`insertAfter` (mutually exclusive) name an *existing* tab to
  position the new one next to; omitting both appends it at the end of the ribbon.
  There is no way to name an existing tab and land content inside it - an extension
  only ever gets its own tab(s), never a foothold in Home/Insert/etc. - matching how
  Microsoft's modern Office Add-ins platform restricts ribbon extensibility to "your
  own tab, positioned next to a built-in one," rather than the older, more permissive
  model of injecting straight into a built-in tab's own groups.

  `groups` is a closed, three-node schema for what a tab may contain - not raw
  notebookbar layout, and not HTML:

  ```json
  "groups": [
    {
      "id": "dateGroup",
      "label": "Date",
      "items": [
        { "type": "button", "command": "insertDate", "size": "large" },
        { "type": "separator" },
        { "type": "button", "command": "insertTime" },
        {
          "type": "menu",
          "title": "Formats",
          "icon": "formats.svg",
          "items": [
            { "command": "insertIsoDate" },
            { "command": "insertLocaleDate" }
          ]
        }
      ]
    }
  ]
  ```

  A **group** (`{ id, label, items }`) is a labeled cluster, like Writer's "Clipboard"
  or "Font" group - it collapses into a dropdown on its own when the window is too
  narrow, the same as every built-in group. Each entry in `items` is one of exactly
  three kinds:
  - **button** - `{ type: "button", command, size?: "large" | "small" }` (default
    `"small"`). `size` chooses a tall icon-over-label button or a compact
    icon-beside-label one. Label and icon come from the referenced command, not from
    the button itself.
  - **separator** - `{ type: "separator" }`, a vertical divider between items in the
    same group. (A separator is also inserted automatically between groups - not
    something you declare.)
  - **menu** - `{ type: "menu", title, icon?, items: [{ command }, ...] }`, a dropdown
    button. `items` is deliberately flat - one level of commands, no nested menu -
    there is no submenu-of-a-submenu case.

  Nothing else is accepted: no raw containers, comboboxes, checkboxes, or other
  layout primitives. A `command` that isn't declared in `contributes.commands`, or an
  item whose `type` isn't one of the three above, is dropped with a console warning
  rather than breaking the rest of the tab.

- `contributes.contextMenu` is a flat list of `{ command, contexts? }` entries
  added to the document's right-click menu:

  ```json
  "contextMenu": [
    { "command": "insertDate" },
    { "command": "makeBold", "contexts": ["text-selection"] }
  ]
  ```

  Every extension that contributes at least one matching entry gets its own
  group at the end of the menu, after a separator - an entry is never
  inserted among the document's own items, the same "own space only" rule
  `contributes.notebookbar` follows for the ribbon. `contexts` (optional;
  omitting it, or passing an empty array, always shows the entry) narrows
  which right-click menu the entry appears in, using one of:
  - `"text-selection"` - shown only when text is selected.
  - `"image"` - shown only when an image or other graphic object is
    selected.

  The existing top-level `supports` field still decides which document
  types load the extension at all, so a Calc-only entry belongs in
  `supports: ["spreadsheet"]`, not in `contexts`.

- `contributes.contextToolbar` is a flat list of `{ command }` entries
  added to the floating toolbar that appears over a mouse text selection:

  ```json
  "contextToolbar": [
    { "command": "makeBold" }
  ]
  ```

  That toolbar only ever shows on desktop, outside read-only mode, outside
  Calc, and only while a selection exists, so there is no `contexts` field
  here the way there is for `contributes.contextMenu` - the toolbar's own
  show logic already covers it. All extensions' buttons share one
  separator after the toolbar's own built-in content, the same "own space,
  appended at the end" rule as the other three contribution points.

- `contributes.keybindings` is a flat list of `{ command, key, modifier? }`
  entries binding a command to a key combination:

  ```json
  "keybindings": [
    { "command": "insertDate", "key": "d", "modifier": ["ctrl", "alt"] }
  ]
  ```

  `modifier` is an array drawn from `"ctrl"`/`"alt"`/`"shift"` - `"ctrl"`
  already means Cmd on macOS, the same as every built-in shortcut. A
  single printable `key` needs at least `"ctrl"` or `"alt"` in `modifier` -
  `"shift"` alone, or no modifier at all, would fight with ordinary
  typing, and is rejected with a console warning. A combination that
  collides with an existing shortcut - built-in, or from another
  extension - is also rejected with a console warning rather than
  registered: two shortcuts sharing the same combination is something the
  underlying keyboard-shortcut dispatch cannot recover from once the key
  is actually pressed, so this is caught up front instead. Pressing the
  combination in read-only (viewing) mode shows the same "you are in
  view mode" attention animation a built-in shortcut shows, rather than
  running the command.

Command ids are namespaced internally so two extensions can never collide.

## Local testing

`make -C browser install-demo-extensions` copies every directory under
`extensions/` into the build's `browser/dist/extensions/`. This only works in an
`--enable-debug` build. For COOL, `extensions/index.json` is synthesized at
runtime, so copying extension directories in directly (without the make
target) works too; for CODA, only the make target writes `index.json`, so
copying directories in directly does not.

Extension discovery itself is gated on the `experimental_features` runtime flag.
