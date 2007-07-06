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
    using FloatingWindow::StateChanged;

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
                         const String& rWndTitle,
                         Window* pParentWindow );
    ~SvxColorWindow_Impl();
    void                StartSelection();

    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );

    virtual SfxPopupWindow* Clone() const;
};

#endif
