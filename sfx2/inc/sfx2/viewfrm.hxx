/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewfrm.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:46:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXVIEWFRM_HXX
#define _SFXVIEWFRM_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#include <sfx2/frame.hxx>
#include <sfx2/shell.hxx>
#include <sfx2/sfxsids.hrc>

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_VERB_HPP_
#include <com/sun/star/frame/status/Verb.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

class SfxMacro;
class SvBorder;
class SfxDispatcher;
class SfxObjectShell;
class SfxBindings;
class SfxProgress;
class SvData;
class SfxViewShell;
class SvPseudoObject;
class SfxCancelManager;
class SystemWindow;
class Fraction;
class Point;
class Size;
class SfxChildWindow;

namespace sfx2
{
class SvLinkSource;
}

#ifndef SFX_DECL_OBJECTSHELL_DEFINED
#define SFX_DECL_OBJECTSHELL_DEFINED
SV_DECL_REF(SfxObjectShell)
#endif

//========================================================================
#include <tools/gen.hxx>
class SvBorder
{
    long nTop, nRight, nBottom, nLeft;
public:
    SvBorder()
    { nTop = nRight = nBottom = nLeft = 0; }
    SvBorder( const Size & rSz )
    { nTop = nBottom = rSz.Height(); nRight = nLeft = rSz.Width(); }
    SvBorder( const Rectangle & rOuter, const Rectangle & rInner );
    SvBorder( long nLeftP, long nTopP, long nRightP, long nBottomP )
    { nLeft = nLeftP; nTop = nTopP; nRight = nRightP; nBottom = nBottomP; }
    BOOL    operator == ( const SvBorder & rObj ) const
            {
                return nTop == rObj.nTop && nRight == rObj.nRight &&
                       nBottom == rObj.nBottom && nLeft == rObj.nLeft;
            }
    BOOL    operator != ( const SvBorder & rObj ) const
            { return !(*this == rObj); }
    SvBorder & operator = ( const SvBorder & rBorder )
            {
                Left()   = rBorder.Left();
                Top()    = rBorder.Top();
                Right()  = rBorder.Right();
                Bottom() = rBorder.Bottom();
                return *this;
            }
    SvBorder & operator += ( const SvBorder & rBorder )
            {
                Left()   += rBorder.Left();
                Top()    += rBorder.Top();
                Right()  += rBorder.Right();
                Bottom() += rBorder.Bottom();
                return *this;
            }
    SvBorder & operator -= ( const SvBorder & rBorder )
            {
                Left()   -= rBorder.Left();
                Top()    -= rBorder.Top();
                Right()  -= rBorder.Right();
                Bottom() -= rBorder.Bottom();
                return *this;
            }
    BOOL    IsInside( const SvBorder & rInside )
            {
                return nTop >= rInside.nTop && nRight >= rInside.nRight &&
                       nBottom >= rInside.nBottom && nLeft >= rInside.nLeft;
            }
    long &  Top()    { return nTop; }
    long &  Right()  { return nRight; }
    long &  Bottom() { return nBottom; }
    long &  Left()   { return nLeft; }
    long    Top()    const { return nTop; }
    long    Right()  const { return nRight; }
    long    Bottom() const { return nBottom; }
    long    Left()   const { return nLeft; }
};
Rectangle & operator += ( Rectangle & rRect, const SvBorder & rBorder );
Rectangle & operator -= ( Rectangle & rRect, const SvBorder & rBorder );


class SFX2_DLLPUBLIC SfxViewFrame: public SfxShell, public SfxListener
{
    struct SfxViewFrame_Impl*   pImp;

    SfxObjectShellRef           xObjSh;
    SfxDispatcher*              pDispatcher;
    SfxBindings*                pBindings;
    USHORT                      nAdjustPosPixelLock;

private:
#ifndef _SFX_HXX
    SAL_DLLPRIVATE void Construct_Impl( SfxObjectShell *pObjSh=NULL );
#endif

protected:
    virtual void            SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                    const SfxHint& rHint, const TypeId& rHintType );

    SAL_DLLPRIVATE void SetWindow_Impl( Window *pWin );

#ifndef _SFX_HXX
    SAL_DLLPRIVATE BOOL SwitchToViewShell_Impl( USHORT nNo, BOOL bIsIndex = FALSE );
    SAL_DLLPRIVATE void KillDispatcher_Impl();
#endif

    virtual                 ~SfxViewFrame();

public:
                            TYPEINFO();
                            SFX_DECL_INTERFACE(SFX_INTERFACE_SFXVIEWFRM)

                            SfxViewFrame( SfxBindings&, SfxFrame*, SfxObjectShell *pDoc=0, sal_uInt32 nType = 0 );
                            SfxViewFrame(SfxObjectShell&, SfxBindings&, SfxFrame*p=0, sal_uInt32 nType = 0);
                            SfxViewFrame(
                                const SfxViewFrame &, SfxBindings &, SfxFrame *pFrame);

    static SfxViewFrame*    SearchViewFrame( SfxViewFrame*, const String& );
    static void             SetViewFrame( SfxViewFrame* );
    static SfxViewFrame*    CreateViewFrame( SfxObjectShell& rDoc,
                                                 USHORT nViewId=0,
                                                 BOOL bHidden=FALSE );

    static SfxViewFrame*    Current();
    static SfxViewFrame*    GetFirst( const SfxObjectShell* pDoc = 0,
                                   TypeId aType = 0,
                                   BOOL bOnlyVisible = TRUE );
    static SfxViewFrame*    GetNext( const SfxViewFrame& rPrev,
                                    const SfxObjectShell* pDoc = 0,
                                    TypeId aType = 0 ,
                                    BOOL bOnlyVisible = TRUE );
    static USHORT           Count(TypeId = 0);

            void            DoActivate(BOOL bMDI, SfxViewFrame *pOld=NULL);
            void            DoDeactivate(BOOL bMDI, SfxViewFrame *pOld=NULL);

    SfxViewFrame*           GetParentViewFrame() const;

    using SfxShell::GetDispatcher;
        SfxDispatcher*          GetDispatcher() { return pDispatcher; }
    SfxBindings&            GetBindings() { return *pBindings; }
    const SfxBindings&      GetBindings() const  { return *pBindings; }
    Window&                 GetWindow() const;
    virtual void            SetZoomFactor( const Fraction &rZoomX,
                                           const Fraction &rZoomY ) = 0;

    SfxProgress*            GetProgress() const;
#ifdef ENABLE_INIMANAGER//MUSTINI
    SfxIniManager*          GetIniManager() const;
#endif

    SfxObjectShell*         GetObjectShell() const
                            { return xObjSh; }

    void                    DoAdjustPosSize( SfxViewShell *pSh,
                                        const Point rPos, const Size &rSize );
    void                    LockAdjustPosSizePixel()
                            { nAdjustPosPixelLock++; }
    void                    UnlockAdjustPosSizePixel()
                            { nAdjustPosPixelLock--; }
    void                    DoAdjustPosSizePixel( SfxViewShell * pSh,
                                        const Point &rPos, const Size &rSize );
    void                    Hide();
    void                    Show();
    BOOL                    IsVisible_Impl() const;
    void                    ToTop();
    void                    Enable( BOOL bEnable );
    virtual BOOL            Close();

    // DDE-Interface
    virtual long            DdeExecute( const String& rCmd );
    virtual long            DdeGetData( const String& rItem,
                                        const String& rMimeType,
                                        ::com::sun::star::uno::Any & rValue );
    virtual long            DdeSetData( const String& rItem,
                                        const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource*   DdeCreateLinkSource( const String& rItem );

    virtual void            ShowStatusText( const String& rText );
    virtual void            HideStatusText();

    virtual String          UpdateTitle();

    // interne Handler
    SAL_DLLPRIVATE virtual BOOL SetBorderPixelImpl( const SfxViewShell *pSh, const SvBorder &rBorder );
    SAL_DLLPRIVATE virtual const SvBorder& GetBorderPixelImpl( const SfxViewShell *pSh ) const;
    SAL_DLLPRIVATE virtual void InvalidateBorderImpl( const SfxViewShell *pSh );

    virtual SfxObjectShell* GetObjectShell();
    USHORT                  GetCurViewId() const;
    SfxFrame*               GetFrame() const;
    SfxViewFrame*           GetTopViewFrame() const;

    BOOL                    DoClose();
    ULONG                   GetFrameType() const
                            { return GetFrame()->GetFrameType(); }
    SfxFrame*               GetTopFrame() const
                            { return GetFrame()->GetTopFrame(); }
    SfxFrame*               SearchFrame( const String& rName, SfxMedium* pMedium = 0)
                            { return GetFrame()->SearchFrame( rName, pMedium ); }
    void                    GetTargetList( TargetList& rList ) const
                            { GetFrame()->GetTargetList( rList ); }
    void                    CancelTransfers()
                            { GetFrame()->CancelTransfers(); }
    SfxCancelManager*       GetCancelManager() const;

    void                    SetModalMode( BOOL );
    BOOL                    IsInModalMode() const;
    void                    Resize(BOOL bForce=FALSE);

    //void                        SetChildWindow(USHORT nId, BOOL bVisible );
    void                        SetChildWindow(USHORT nId, BOOL bVisible, BOOL bSetFocus=TRUE);
    void                        ToggleChildWindow(USHORT);
    BOOL                        HasChildWindow(USHORT);
    BOOL                        KnowsChildWindow(USHORT);
    void                        ShowChildWindow(USHORT,BOOL bVisible=TRUE);
    SfxChildWindow*             GetChildWindow(USHORT);
    void                        ChildWindowExecute(SfxRequest&);
    void                        ChildWindowState(SfxItemSet&);

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE SfxMacro* GetRecordingMacro_Impl();
    SAL_DLLPRIVATE void SetFrame_Impl( SfxFrame* );
    SAL_DLLPRIVATE void SetDowning_Impl();
    SAL_DLLPRIVATE void GetDocNumber_Impl();
    SAL_DLLPRIVATE BOOL IsDowning_Impl() const;
    SAL_DLLPRIVATE void SetSetViewFrameAllowed_Impl( BOOL bSet );
    SAL_DLLPRIVATE BOOL IsSetViewFrameAllowed_Impl() const;
    SAL_DLLPRIVATE void SetImportingObjectShell_Impl( SfxObjectShell* pSH );
    SAL_DLLPRIVATE SfxObjectShell* GetImportingObjectShell_Impl( ) const;
    SAL_DLLPRIVATE void SetViewShell_Impl( SfxViewShell *pVSh );
    SAL_DLLPRIVATE void SetObjectShell_Impl( SfxObjectShell& rObjSh ,
                                                 FASTBOOL bDefaultView = FALSE );
    SAL_DLLPRIVATE void ReleaseObjectShell_Impl( BOOL bStoreView = FALSE );

    SAL_DLLPRIVATE void GetState_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecReload_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void ExecReload_Impl( SfxRequest &rReq, BOOL bAsync );
    SAL_DLLPRIVATE void StateReload_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecView_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateView_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void PropState_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void ExecHistory_Impl( SfxRequest &rReq );
    SAL_DLLPRIVATE void StateHistory_Impl( SfxItemSet &rSet );
    SAL_DLLPRIVATE void SetParentViewFrame_Impl(SfxViewFrame *pParentFrame);
    SAL_DLLPRIVATE SfxViewFrame* GetParentViewFrame_Impl() const;
    SAL_DLLPRIVATE void ForceOuterResize_Impl(BOOL bOn=TRUE);
    SAL_DLLPRIVATE BOOL IsResizeInToOut_Impl() const;
    SAL_DLLPRIVATE BOOL IsAdjustPosSizePixelLocked_Impl() const
                            { return nAdjustPosPixelLock != 0; }
    SAL_DLLPRIVATE void ForceInnerResize_Impl( BOOL bOn );
    SAL_DLLPRIVATE void UpdateDocument_Impl();

    SAL_DLLPRIVATE void LockObjectShell_Impl(BOOL bLock=TRUE);

    SAL_DLLPRIVATE SfxViewShell* CreateView_Impl( USHORT nViewId );
    SAL_DLLPRIVATE void MakeActive_Impl( BOOL bActivate );
    SAL_DLLPRIVATE void SetQuietMode_Impl( BOOL );
    SAL_DLLPRIVATE const Size& GetMargin_Impl() const;
    SAL_DLLPRIVATE void SetMargin_Impl( const Size& );
    SAL_DLLPRIVATE void SetActiveChildFrame_Impl( SfxViewFrame* );
    SAL_DLLPRIVATE SfxViewFrame* GetActiveChildFrame_Impl() const;
    SAL_DLLPRIVATE BOOL IsRestoreView_Impl() const;
    SAL_DLLPRIVATE void SetRestoreView_Impl( BOOL );
    SAL_DLLPRIVATE void SetViewData_Impl( USHORT, const String& );
    SAL_DLLPRIVATE String& GetViewData_Impl();
    SAL_DLLPRIVATE String GetActualPresentationURL_Impl() const;
    SAL_DLLPRIVATE static void CloseHiddenFrames_Impl();
    SAL_DLLPRIVATE void MiscExec_Impl(SfxRequest &);
    SAL_DLLPRIVATE void MiscState_Impl(SfxItemSet &);
    SAL_DLLPRIVATE SfxWorkWindow* GetWorkWindow_Impl( USHORT nId );
    SAL_DLLPRIVATE void AddDispatchMacroToBasic_Impl(const ::rtl::OUString& sMacro);
    SAL_DLLPRIVATE BOOL ClearEventFlag_Impl();
//#endif
};

//--------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxViewFrameItem: public SfxPoolItem
{
    SfxViewFrame*           pFrame;

public:
                            TYPEINFO();
                            SfxViewFrameItem( SfxViewFrame *pViewFrame ):
                                SfxPoolItem( 0 ),
                                pFrame( pViewFrame)
                            {}
                            SfxViewFrameItem( USHORT nWhichId, SfxViewFrame *pViewFrame ):
                                SfxPoolItem( nWhichId ),
                                pFrame( pViewFrame)
                            {}

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual String          GetValueText() const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    SfxViewFrame*           GetFrame() const
                            { return pFrame; }
};

class SfxVerbListItem : public SfxPoolItem
{
    com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor > aVerbs;

public:
                            TYPEINFO();
                            SfxVerbListItem( USHORT nWhichId = SID_OBJECT ) :
                                SfxPoolItem( nWhichId )
                            {}

                            SfxVerbListItem( USHORT nWhichId, const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& );

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    const com::sun::star::uno::Sequence < com::sun::star::embed::VerbDescriptor >& GetVerbList() const { return aVerbs; }
};

#endif
