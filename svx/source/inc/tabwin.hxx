/*************************************************************************
 *
 *  $RCSfile: tabwin.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-29 10:23:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

//==================================================================
class FmFieldWin;
class FmFieldWinListBox
                    :public SvTreeListBox
{
    FmFieldWin* pTabWin;

protected:
//  virtual void Command( const CommandEvent& rEvt );

public:
    FmFieldWinListBox( FmFieldWin* pParent );
    virtual ~FmFieldWinListBox();

    virtual sal_Bool QueryDrop( DropEvent& rDEvt )
    {
        return (!nDragDropMode) ? sal_False : SvTreeListBox::QueryDrop(rDEvt);
    }
        // this prevents an assertion the base class makes (unjustified ?) to fail - 67071 - fs - 21.06.99

protected:
    // DragSourceHelper
    virtual void StartDrag( sal_Int8 nAction, const Point& rPosPixel );
};

//========================================================================
class FmFormShell;
class FmFieldWinData
{
public:
    FmFieldWinData();
    ~FmFieldWinData();
};

//========================================================================
class FmFieldWin    :public SfxFloatingWindow
                    ,public SfxControllerItem
                    ,public ::comphelper::OPropertyChangeListener
{
    ::osl::Mutex        m_aMutex;
    FmFieldWinListBox* pListBox;
    FmFieldWinData*    pData;
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
    virtual void StateChanged(sal_uInt16 nSID, SfxItemState eState,
                              const SfxPoolItem* pState);

    FmFieldWinData* GetData()  const {return pData;}

    sal_Bool Update(FmFormShell*);
    sal_Bool Update(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm > &);
    void FillInfo( SfxChildWinInfo& rInfo ) const;

    const ::rtl::OUString&  GetDatabaseName() const { return m_aDatabaseName; }
    const ::rtl::OUString&  GetObjectName() const { return m_aObjectName; }
    sal_Int32               GetObjectType() const { return m_nObjectType; }

protected:
    // FmXChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw( ::com::sun::star::uno::RuntimeException );
};

//========================================================================
class FmFieldWinMgr : public SfxChildWindow
{
public:
    FmFieldWinMgr(Window *pParent, sal_uInt16 nId,
        SfxBindings *pBindings, SfxChildWinInfo *pInfo);
    SFX_DECL_CHILDWINDOW(FmFieldWinMgr);
};


#endif

