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

class SvxColorWindow_Impl : public SfxPopupWindow, public SfxListener
{
private:
    const USHORT    theSlotId;
    ValueSet        aColorSet;

#if _SOLAR__PRIVATE
    DECL_LINK( SelectHdl, void * );
#endif

protected:
    virtual void    Resize();
    virtual BOOL    Close();

public:
    SvxColorWindow_Impl( USHORT nId, USHORT nSlotId,
                    const String& rWndTitle,
                    SfxBindings& rBindings );
    ~SvxColorWindow_Impl();
    void            StartSelection();

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );
    virtual void        KeyInput( const KeyEvent& rKEvt );

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
                SvxTbxButtonColorUpdater_Impl( USHORT nTbxBtnId,
                                          ToolBox* ptrTbx, USHORT nMode = 0 );
                ~SvxTbxButtonColorUpdater_Impl();

    void        Update( const Color& rColor );

protected:
    void        DrawChar(VirtualDevice&, const Color&);

private:
    USHORT      mnDrawMode;
    USHORT      mnBtnId;
    ToolBox*    mpTbx;
    Color       maCurColor;
    Rectangle   maUpdRect;
    Size        maBmpSize;
    BOOL        mbWasHiContrastMode;
};

#endif
