Visual Formula Editor for LibreOffice Math
==========================================

This repository facilitates development of a visual formula editor
for LibreOffice/OpenOffice Math. This work was started by me (Jonas
 Finnemann Jensen) during GSoC 2010 with Go-OO. During development
this repository will be merged with libreoffice/writer on a random
basis, but will hopefully at some point be merged back into
LibreOffice.

Description of the GSoC project that started this, be found
[here](http://www.freedesktop.org/wiki/Software/ooo-build/SummerOfCode/2010/Jonas).
Please note that this page will not be updated futher, but it does
contain relevant information about the project and discusses the
implementation strategy. I might also post progress updates on
[my blog](http://jopsen.dk/blog/category/computer/openoffice/).
But the project status will be documented and updated in this file, see below.

Videos of the patch in action can be found at:

 * [Development Preview 3 (13th of August)](http://www.youtube.com/watch?v=3foNqKYAlYY)
 * [Development Preview 2 (26th of July)](http://www.youtube.com/watch?v=tELPgJIC1sg)
 * [Development Preview 1 (21th of July)](http://www.youtube.com/watch?v=W8yXyDiIQPc)

(All videos can also be found [here](http://jopsen.dk/downloads/GSoC2010/Videos/) better quality and various formats).

If you're interested in more technical details the source (that I've
written) is extensively documented with doxygen comments. You're also
welcome to [contact me](http://jopsen.dk/blog/about/) with any
questions, or wish to help hacking. My email is <jopsen@gmail.com>
and I'm jopsen on #LibreOffice (if I'm online), there's also a finite
set of [other options](http://jopsen.dk/blog/about/).

Project Status
==============
*This is basically my todo list, so don't be surprised if things are not immediately obvious.*

Easy
----
1. Draw a non-blinking line under the the visual line that the caret is in, in `SmCaretDrawingVisitor`.
2. `SmGraphicWindow::KeyInput` relies on comparison of `sal_Char`, a better way must be available for CTRL+c
3. Code style (missing spaces, linebreaks and a few renames)
4. More documentation
5. Replace `j_assert` with `DBG_ASSERT`

Medium
------
1. `SmCursor::InsertCol()` method for added columns to matrices should be implemented.
2. `SmCursor` should support deletion of lines, rows, cols and sub-/superscripts.
3. `SmCursor::InsertSubSup()` should wrap the body in a `SmBraceNode` if the body is an `SmOperNode`, `SmBinVerNode`, etc.
4. Make caret in visual editor blink.
5. Don't draw visual editor caret, when `SmGraphicWindow` doesn't have focus.
6. When OpenOffice Math runs in standalone mode it centers the current formula, this is not nice for visual editing.

Complex
-------
1. `SmAlignNode` and `SmFontNode` are ignored by visual editor, figure out how these should work.
2. Solve the flickering issue when drawing formulas (See e-mail)
3. Make " a shortcut for creating an `SmTextNode` with `FNT_TEXT`, also check that `SmNodeToTextVisitor` supports this.
4. `parse.cxx` merges multiple blanks into one `SmBlankNode`, the visual editor doesn't...

Complex and non-essential
-------------------------
1. Global clipboard integration
2. Support undo/redo with `UndoManager` integration
3. Consider improving GUI for "Formula Elements"-dialog, most buttons work with visual editor
4. Consider allowing users to enter commands in visual editor, by prefixing the command...
5. Optimize things, for instance `SmCursor::AnnotateSelection()` is called way too many places...
6. Improve handling of `MoveUp` and `MoveDown` in `SmCursor::Move`, `SmCaretPos2LineVisitor` might need improvement.

Items are organized by complexity and importance, and I'm not sure everything needs to be addressed
before release, but it constitutes a list of things I can work on.
