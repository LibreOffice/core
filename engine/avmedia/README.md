# Audio / video Media Implementation.

Provides the media framework: embedding of media files into the
document, extraction to temp files, and the `MediaWindow` API the
applications call to insert media. Currently no stream API is provided,
only a URI based one, so streaming has to be wrapped around it via temp
files.

Also provides (in `source/framework/mediacontrol.cxx`) an implementation
of the graphical media playback control that appears in the toolbar /
mediaobject bar when media is selected under the `.uno:AVMediaToolBox`
item.

There is no platform implementation of the `com.sun.star.media.Manager`
service; playback is done by the client.

## Future Works

it might be worthwhile to revamp the avmedia UI.
