# Audio / video Media Implementation.

Provides per-platform implementations of multimedia functionality.
Currently no stream API is provided, only a URI based one, so
streaming has to be wrapped around it via temp files.

Also provides (in `source/framework/mediacontrol.cxx`) an implementation
of the graphical media playback control that appears in the toolbar /
mediaobject bar when media is selected under the `.uno:AVMediaToolBox`
item.

## avmedia / gstreamer

The `avmedia` component is implementation of manager service defined in
`offapi/com/sun/star/media/`. Radek has added implementation based on
gstreamer so that we can add audio and video files into impress
presentation on Linux with gstreamer.

The implementation is pretty straightforward, sometimes it has
problems when gstreamer installation is incomplete.

In the beginning the media files were not embedded, Thorsten added
support for that later.

## Future Works

it might be worthwhile to revamp the avmedia UI.
