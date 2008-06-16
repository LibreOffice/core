Extension identifiers

Extension identifiers have been introduced because of the new filename-
extension (.oxt) might lead to conflicts with older extensions of the same name.
Previously extensions were identified by their filenames only which is 
insufficient. Old extensions get an automatic legacy identifier and from now on
an identifier can be explicitly set.

This directory contains two extensions, one being the legacy and the other one
having the identifier explicitly set. You should be able to install them both
at the same time.

This feature is tested by framework\basic\f_basic_extension_identifiers.bas.
The spec can be found here: 
http://specs.openoffice.org/appwide/packagemanager/extensionidentifiers.odt
The feature was brought in by CWS sb66, the issue is #i69910
