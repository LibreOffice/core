# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import officehelper
import sys
import traceback


FIRST_PARAGRAPH = (
    "He heard quiet steps behind him. That didn't bode well. Who could be following "
    "him this late at night and in this deadbeat part of town? And at this "
    "particular moment, just after he pulled off the big time and was making off "
    "with the greenbacks. Was there another crook who'd had the same idea, and was "
    "now watching him and waiting for a chance to grab the fruit of his labor?"
)

SECOND_PARAGRAPH = (
    "Or did the steps behind him mean that one of many bloody officers in town was "
    "on to him and just waiting to pounce and snap those cuffs on his wrists? He "
    "nervously looked all around. Suddenly he saw the alley. Like lightning he "
    "darted off to the left and disappeared between the two warehouses almost "
    "falling over the trash can lying in the middle of the sidewalk. He tried to "
    "nervously tap his way along in the inky darkness and suddenly stiffened: it was "
    "a dead-end, he would have to go back the way he had come"
)

THIRD_PARAGRAPH = (
    "The steps got louder and louder, he saw the black outline of a figure coming "
    "around the corner. Is this the end of the line? he thought pressing himself "
    "back against the wall trying to make himself invisible in the dark, was all "
    "that planning and energy wasted? He was dripping with sweat now, cold and wet, "
    "he could smell the brilliant fear coming off his clothes. Suddenly next to him, "
    "with a barely noticeable squeak, a door swung quietly to and fro in the night's "
    "breeze."
)


def create_example_text(component):
    """Create example text

    :param component: object which implements com.sun.star.text.XTextDocument interface.
    """
    try:
        cursor = component.getText().createTextCursor()
        cursor.setString(FIRST_PARAGRAPH)
        cursor.collapseToEnd()
        cursor.setString(SECOND_PARAGRAPH)
        cursor.collapseToEnd()
        cursor.setString(THIRD_PARAGRAPH)
        cursor.gotoStart(False)
    except Exception:
        traceback.print_exc()


def find_first(document, search_str):
    """Find the text

    :param document: object which implements com.sun.star.text.XTextDocument interface.
    :param str search_str: the search string.
    :return: object representing the searched text, which implements com.sun.star.text.XTextRange interface.
    :rtype: com.sun.star.uno.XInterface
    """
    try:
        descriptor = document.createSearchDescriptor()
        descriptor.setSearchString(search_str)
        descriptor.setPropertyValue("SearchRegularExpression", True)
        return document.findFirst(descriptor)
    except Exception:
        traceback.print_exc()
        return None


def insert_bookmark(document, text_range, bookmark_name):
    """Insert bookmark

    :param document: object which implements om.sun.star.text.XTextDocument interface.
    :param text_range: object representing the text range bookmark is inserted for.
        This object should implement com.sun.star.text.XTextRange interface.
    :param str bookmark_name: bookmark name.
    """
    try:
        bookmark = document.createInstance("com.sun.star.text.Bookmark")
        bookmark.setName(bookmark_name)
        document.getText().insertTextContent(text_range, bookmark, True)
        print("Insert bookmark:", bookmark_name)
    except Exception:
        traceback.print_exc()


def mark_list(component, mlist, prefix):
    """Mark the matched text

    :param component: object which implements com.sun.star.text.XTextDocument interface.
    :param list[str] mlist: list of patterns to search text from document.
    :param str prefix: prefix used to construct bookmark name.
    """
    try:
        for i, search_str in enumerate(mlist):
            search = find_first(component, search_str)
            if not search:
                continue
            insert_bookmark(component, search, f"{prefix}{i}")
    except Exception:
        traceback.print_exc()
        sys.exit(1)


def get_desktop():
    desktop = None
    try:
        remote_context = officehelper.bootstrap()
        srv_mgr = remote_context.getServiceManager()
        if srv_mgr is None:
            print("Can't create a desktop. No connection, no remote office servicemanager available!")
        else:
            desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)
            print("Connected to a running office ...")
    except Exception:
        traceback.print_exc()
        sys.exit(1)
    return desktop


def main():
    desktop = get_desktop()
    if desktop is None:
        return

    # Open an empty text document.
    try:
        doc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, tuple([]))
    except Exception:
        traceback.print_exc()
        sys.exit(1)

    create_example_text(doc)

    mOffending = ["negro(e|es)?", "bor(ed|ing)?", "bloody?", "bleed(ing)?"]
    mBad = ["possib(le|ilit(y|ies))", "real(ly)+", "brilliant"]

    sOffendPrefix = "Offending"
    sBadPrefix = "BadStyle"

    mark_list(doc, mOffending, sOffendPrefix)
    mark_list(doc, mBad, sBadPrefix)

    print("Done")


if __name__ == "__main__":
    main()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
