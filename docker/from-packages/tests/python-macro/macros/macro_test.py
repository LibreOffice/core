# Share-location Python macro used by the container test. It does what the
# reporting user wants to do: enable track changes programmatically, then
# insert a marker string the test can detect (which becomes a tracked change).
import uno  # noqa: F401 - importing pyuno is part of what is being tested


def enable_track_changes(*args):
    doc = XSCRIPTCONTEXT.getDocument()  # noqa: F821 - provided by pythonscript
    doc.setPropertyValue("RecordChanges", True)
    text = doc.getText()
    cursor = text.createTextCursorByRange(text.getStart())
    text.insertString(cursor, "PYMACRO_TRACKED_OK", False)
    return "done"


g_exportedScripts = (enable_track_changes,)
