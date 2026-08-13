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

Lets an extension put commands directly into the classic menu, without needing its
sidebar panel (if it has one) to ever be opened.
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
    { "id": "insertDate", "title": "Insert Date", "script": "commands/dates.js" }
  ],
  "menus": {
    "insert": ["insertDate"]
  }
}
```

- `contributes.commands` - each entry's `script` is a path (relative to the manifest)
  to a JS file whose top-level binding named `commands` is an object mapping
  command ids to functions, e.g.:

  ```js
  var commands = {
      insertDate: function () {
          var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
          var model = desktop.getCurrentFrame().getController().getModel();
          // ... call into the document via the UNO API ...
      },
  };
  ```

  More than one command may name the same `script` file - add another key to its
  `commands` object rather than giving each command a one-function file of its own.
  A referenced function runs when its command is chosen from the menu. It only has
  access to the UNO API shown above - no DOM, no network, no other browser
  capability - and it runs without ever loading a visible panel, so the command
  works whether or not the extension has a sidebar UI at all.

- `contributes.menus` maps an existing top-level classic-menu id (`file`, `editmenu`,
  `view`, `insert`, `format`, and so on, depending on the document type) to a list of
  command ids appended to the end of that menu.

Command ids are namespaced internally so two extensions can never collide.

## Local testing

`make -C browser install-demo-extensions` copies every directory under
`extensions/` into the build's `browser/dist/extensions/`. This only works in an
`--enable-debug` build. For COOL, `extensions/index.json` is synthesized at
runtime, so copying extension directories in directly (without the make
target) works too; for CODA, only the make target writes `index.json`, so
copying directories in directly does not.

Extension discovery itself is gated on the `experimental_features` runtime flag.
