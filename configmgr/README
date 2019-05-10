UNO services to access the configuration database

== Functional Overview ==

This code parses the settings that are described in the [[officecfg]]
directory, and provides a UNO API that code can use to set and get
settings.

== Source Overview ==

configurationprovider.cxx
configurationregistry.cxx
defaultprovider.cxx
services.cxx
 UNO service implementations.

access.cxx
childaccess.cxx
rootaccess.cxx
 UNO objects passed to clients.

components.cxx
 Central singleton that aggregates all data (reads in the XML files, manages
 modifications and global notifications).

groupnode.cxx
localizedpropertynode.cxx
localizedvaluenode.cxx
node.cxx
propertynode.cxx
setnode.cxx
 Internal representations of data nodes.

parsemanager.cxx
parser.hxx
valueparser.cxx
xcdparser.cxx
xcsparser.cxx
xcuparser.cxx
xmldata.cxx
 XML file reading.

modifications.cxx
writemodfile.cxx
 Modification management.

broadcaster.cxx
 Notification management.

additions.hxx
update.cxx
 Extension manager interface.

data.cxx
lock.cxx
nodemap.cxx
partial.cxx
path.hxx
type.cxx
 Utilities.


== Some Implementation Notes ==

=== Mandatory Set Members ===

- A set member can be marked as "mandatory," meaning that a member of that name
must always be present in a set.

- The above definition implies that calling replaceByName on a mandatory set
member is OK.

- The XCU format can contain oor:mandatory attributes on nodes.  (The XCS format
does not support them.  In the registrymodifications file, oor:mandatory
attributes should never be needed, as being mandatory cannot be controlled via
the UNO API.)  The XCU reading code only evaluates the oor:mandatory attribute
for set members, and ignores it everywhere else.

- Only true sets support mandatory members.  A localized property for the "*"
locale, though acting much like a set, does not support mandatory members.

- The OpenOffice.org Registry Format document claims that group extension
properties are implicitly mandatory, but at least the new configmgr code does
not treat them like that (i.e., they can be removed again).

- For simplicity, setMandatory/getMandatory are available as virtual functions
at the base Node, even though they can only make sense for GroupNodes and
SetNodes that are set members.  The default getMandatory implementation returns
NO_LAYER, meaning oor:mandatory is not set to true in any layer.  (Returning
NO_LAYER simplifies the code, e.g., removeByName does not have to check whether
getMandatory is called on a member of a true set to decide whether to forbid
removal.)

- When committing changes (made through the UNO API), the "mandatory" status of
inserted nodes must be updated (in case the insert is due to a replaceByName, or
the "mandatory" flag was added by a concurrent modification of a lower layer).
Also, for to-be-removed nodes, removal is ignored for (newly; due to concurrent
modification of a lower layer) mandatory nodes (but still recorded in
registrymodifications, so may take effect once the lower layer addition is
removed again---whether or not that is a good idea).


=== XcuParser Modification Recording ===

- XcuParser records modifications when reading user layer data
(valueParser_.getLayer() == Data::NO_LAYER).

- oor:finalized and oor:mandatory attributes cannot be set via the UNO API, so
it is assumed that user layer data does not contain them (for one, they are not
written by writeModFile; for another, the logic to record modifications expects
a locprop(modify,fuse) to be followed by one or more value(fuse,remove), which
would not necessarily be true if the locprop were only present in the user layer
data to flag it as finalized).

- The logic to record modifications considers the top of the XML element stack.
In the following list of all possible cases, items marked with an asterisk are
recorded:
 ... group(modify,fuse) - group(modify,fuse) - ...
 ... group(modify,fuse) - set(modify,fuse) - ...
 ... group(modify,fuse) - *prop(modify,fuse,replace) - value(fuse)
 ... group(modify,fuse) - *prop(remove)
 ... group(modify,fuse) - locprop(modify,fuse) - *value(fuse)
 ... group(modify,fuse) - locprop(modify,fuse) - *value(remove)
 ... group(modify,fuse) - *locprop(replace) ...
 ... set(modify,fuse,replace) - group(modify/fuse) - ...
 ... set(modify,fuse,replace) - *group(replace/fuse) - ...
 ... set(modify,fuse,replace) - *group(remove)
 ... set(modify,fuse,replace) - set(modify/fuse) - ...
 ... set(modify,fuse,replace) - *set(replace/fuse) - ...
 ... set(modify,fuse,replace) - *set(remove)
Legend: "...": zero or more further items
        "- ...": one or more further items
        "modify,fuse" etc.: any of those operations
        "modify/fuse": a modify or a fuse on an existing member
        "replace/fuse": a replace or a fuse on a non-existing member

