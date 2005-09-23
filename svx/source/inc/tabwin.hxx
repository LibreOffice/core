/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabwin.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 12:00:35 $
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
#ifndef _SVX_TABWIN_HXX
#define _SVX_TABWIN_HXX

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif

//#ifndef _SVX_FMTOOLS_HXX
//#include "fmtools.hxx"
//#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef SVX_DBTOOLSCLIENT_HXX
#include "dbtoolsclient.hxx"
#endif

//==================================================================
class FmFieldWin;
class SAL_DLLPRIVATE FmFieldWinListBox
                    :public SvTreeListBox
{
    FmFieldWin* pTabWin;

protected:
//  virtual void Command( const CommandEvent& rEvt );

public:
    FmFieldWinListBox( FmFieldWin* pParent );
    virtual ~FmFieldWinListBox();

    sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );
    sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

protected:
    // DragSourceHelper
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // SvLBox
    virtual BOOL DoubleClickHdl();
};

//========================================================================
class FmFormShell;
class SAL_DLLPRIVATE FmFieldWinData
{
public:
    FmFieldWinData();
    ~FmFieldWinData();
};

//========================================================================
class SAL_DLLPRIVATE FmFieldWin :public SfxFloatingWindow
                    ,public SfxControllerItem
                    ,public ::comphelper::OPropertyChangeListener
                    ,public ::svxform::OStaticDataAccessTools
{
    ::osl::Mutex        m_aMutex;
    FmFieldWinListBox* pListBox;
    FmFieldWinData*    pData;
    ::svxform::SharedConnection
                       m_aConnection;
    ::rtl::OUString    m_aDatabaseName,
                       m_aObjectName;
    sal_Int32          m_nObjectType;

    ::comphelper::OPropertyChangeMultiplexer*   m_pChangeListener;

public:
    FmFieldWin(SfxBindings *pBindings,
               SfxChildWindow *pMgr, Window* pParent);

    virtual ~FmFieldWin();
    virtual void Resize();
    virtual sal_Bool Close();
    virtual void GetFocus();
    virtual long PreNotify( NotifyEvent& _rNEvt );
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState);

    FmFieldWinData* GetData()  const {return pData;}

    sal_Bool Update(FmFormShell*);
    sal_Bool Update(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > &);
    void FillInfo( SfxChildWinInfo& rInfo ) const;

    const ::rtl::OUString&      GetDatabaseName() const { return m_aDatabaseName; }
    ::svxform::SharedConnection GetConnection() const { return m_aConnection; }
    const ::rtl::OUString&      GetObjectName() const { return m_aObjectName; }
    sal_Int32                   GetObjectType() const { return m_nObjectType; }

    sal_Bool    createSelectionControls( );

protected:
    // FmXChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );

protected:
    inline          SfxBindings&    GetBindings()       { return SfxControllerItem::GetBindings(); }
    inline  const   SfxBindings&    GetBindings() const { return SfxControllerItem::GetBindings(); }
};

//========================================================================
class SAL_DLLPRIVATE FmFieldWinMgr : public SfxChildWindow
{
public:
    FmFieldWinMgr(Window *pParent, sal_uInt16 nId,
        SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmFieldWinMgr);
};


#endif

