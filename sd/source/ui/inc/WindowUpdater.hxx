#ifndef SD_OUTPUT_DEVICE_UPDATER_HXX
#define SD_OUTPUT_DEVICE_UPDATER_HXX

#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <svtools/ctloptions.hxx>
#endif
#include <vector>

class SdViewShell;
class Window;
class OutputDevice;
class SdDrawDocument;


namespace sd {

/** The purpose of the <type>WindowUpdater</type> is to update output
    devices to take care of modified global values.  These values are
    monitored for changes.  At the moment this is
    the digit language that defines the glyphs to use to render digits.
    Other values may be added in the future.

    <p>The methods of this class have not been included into the
    <type>SdViewShell</type> class in order to not clutter its interface any
    further.  This class accesses some of <type>SdViewShell</type> data
    members directly and thus is declared as its friend.</p>

    <p>Windows that are to be kept up-to-date have to be registered via the
    <member>RegisterWindow()</member> method.  After setting a view shell
    with the <member>SetViewShell()</type> method you can use the
    convenience method <member>RegisterPreview()</member> to register the
    preview of the view shell.  When a document is given then this document
    is reformatted when the monitored option changes.</p>
*/
class WindowUpdater
    : public SfxListener
{
public:
    explicit WindowUpdater (void);
    virtual ~WindowUpdater (void) throw();

    /** Add the given device to the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is added to the device list if it is not <null/> and
            when it is not already a member of that list.
    */
    void RegisterWindow (Window* pWindow);

    /** Remove the given device from the list of devices which will be updated
        when one of the monitored values changes.
        @param pWindow
            This device is removed from the device list when it is a member
            of that list.
    */
    void UnregisterWindow (Window* pWindow);

    /** Set the view shell whose output devices shall be kept up to date.
        It is used to access the preview windows and to clear the master
        page cache so that a redraw affects the master page content as
        well.
    */
    void SetViewShell (SdViewShell& rViewShell);

    /** Set the document so that it is reformatted when one of the monitored
        values changes.
        @param pDocument
            When <null/> is given document reformatting will not take
            place in the future.
    */
    void SetDocument (SdDrawDocument* pDocument);

    /** Convenience method that retrieves the output device of the preview
        window of the view shell and then registers this device to be
        updated.
    */
    void RegisterPreview (void);

    /** Convenience method that retrieves the output device of the preview
        window of the view shell and then un-registers this device.
    */
    void UnregisterPreview (void);

    /** Update the given output device and update all text objects of the
        view shell if not told otherwise.
        @param pWindow
            The device to update.  When the given pointer is NULL then
            nothing is done.
        @param pDocument
            When given a pointer to a document then tell it to reformat all
            text objects.  This refromatting is necessary for the new values
            to take effect.
    */
    void Update (OutputDevice* pDevice, SdDrawDocument* pDocument=0) const;

    /** Callback that waits for notifications of a
        <type>SvtCTLOptions</type> object.
    */
    virtual void Notify (SfxBroadcaster& rBC, const SfxHint& rHint);

private:
    /// Options to monitor for changes.
    SvtCTLOptions maCTLOptions;

    /// Keep the output devices of this view shell up to date.
    SdViewShell* mpViewShell;

    /// The document rendered in the output devices.
    SdDrawDocument* mpDocument;

    /// Copy constructor not supported.
    WindowUpdater (const WindowUpdater& pViewShell);

    /// Assignment operator not supported.
    WindowUpdater operator= (const WindowUpdater& rViewShell);

    /** Type and data member for a list of devices that have to be kept
        up-to-date.
    */
    typedef ::std::vector<Window*> tWindowList;
    tWindowList maWindowList;

    /** The central method of this class.  Update the given output device.
        It is the task of the caller to initiate a refrormatting of the
        document that is rendered on this device to reflect the changes.
        @param pWindow
            The output device to update.  When it is <null/> then the call
            is ignored.
    */
    void UpdateWindow (OutputDevice* pDevice) const;

    /** Used by <member>RegisterPreview()</member> and
        <member>UnregisterPreview()</member> this method returns the output
        device of the current preview window of the view shell.
        @return
            Returns <null/> when there is no preview window or no view shell
            has been set from which the preview can be retrieved.
    */
    Window* GetPreviewWindow (void) const;
};

} // end of namespace sd

#endif
