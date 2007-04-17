The updates, that is the newer versions, are defect. However, only fail2.oxt fails to install. The other extensions can be installed directly and through an update.

fail1.oxt: in version2 the contained t.rdb was renamed so that it is not found (t.rdb is referenced in the manifest.xml).

fail2.oxt: in version 2 the contained t.rdb is corrupted. It is a renamed .txt file which contains some text.

fail3.oxt: in version 2 the contained t.rdb is corrupted. It is a renamed .txt file which does not contain any text.

fail4.oxt: the version 2 references by fail4.update.xml is empty.

info1.oxt: The update information file has length null.

info2.oxt: The update information does not contain xml.

info3.oxt: The update information contain an error: the tag update information contains two opening brackets (<<update-information>)