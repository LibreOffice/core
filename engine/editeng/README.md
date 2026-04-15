# Edit Engine

In OpenOffice.org build DEV300m72 this module was split off from `svx` but it
has no dependencies on `svx` (nor on `sfx2`) while in turn `svx` depends on editeng

Read more in the mailing list post:
<http://www.mail-archive.com/dev@openoffice.org/msg13237.html>

If you build LibreOffice with `dbgutil`, you have some extended debug keys:

- Ctrl+Alt+F1 - draws the paragraph rectangles in different colors
- Ctrl+Alt+F11 - toggles dumping the edit engine state to the
               "editenginedump.log" on draw
- Ctrl+Alt+F12 - dumps the current edit engine state to "editenginedump.log"
