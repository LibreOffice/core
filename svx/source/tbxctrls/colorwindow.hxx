//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================
#ifndef __SVX_COLORWINDOW_HXX_
#define __SVX_COLORWINDOW_HXX_

#ifndef _SFXTBXCTRL_HXX
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef _VALUESET_HXX
#include <vcl/valueset.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

//========================================================================
// class SvxColorWindow_Impl --------------------------------------------------
//========================================================================

class SvxColorWindow_Impl : public SfxPopupWindow
{
private:
    const USHORT                                                        theSlotId;
    ValueSet                                                            aColorSet;
    rtl::OUString                                                       maCommand;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual BOOL    Close();

public:
    SvxColorWindow_Impl( const rtl::OUString& rCommand,
                         USHORT nSlotId,
                         const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                         const String& rWndTitle );
    ~SvxColorWindow_Impl();
    void                StartSelection();

    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

    virtual SfxPopupWindow* Clone() const;
};

//========================================================================
// class SvxTbxButtonColorUpdater_Impl ----------------------------------------
//========================================================================
#define TBX_UPDATER_MODE_NONE               0x00
#define TBX_UPDATER_MODE_CHAR_COLOR         0x01
#define TBX_UPDATER_MODE_CHAR_BACKGROUND    0x02
#define TBX_UPDATER_MODE_CHAR_COLOR_NEW     0x03

class SvxTbxButtonColorUpdater_Impl
{
public:
                SvxTbxButtonColorUpdater_Impl( USHORT   nSlotId,
                                               USHORT   nTbxBtnId,
                                               ToolBox* ptrTbx,
                                               USHORT   nMode = 0 );
                ~SvxTbxButtonColorUpdater_Impl();

    void        Update( const Color& rColor );

protected:
    void        DrawChar(VirtualDevice&, const Color&);

private:
    USHORT      mnDrawMode;
    USHORT      mnBtnId;
    USHORT      mnSlotId;
    ToolBox*    mpTbx;
    Color       maCurColor;
    Rectangle   maUpdRect;
    Size        maBmpSize;
    BOOL        mbWasHiContrastMode;
};

#endif
