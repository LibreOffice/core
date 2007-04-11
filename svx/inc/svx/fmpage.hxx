/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmpage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:51:37 $
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

#ifndef _SVX_FMPAGE_HXX
#define _SVX_FMPAGE_HXX

#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif

#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class StarBASIC;
class FmFormModel;
class FmFormPageImpl;   // haelt die Liste aller Forms

FORWARD_DECLARE_INTERFACE(container,XNameContainer)

class SdrView;
class SfxJSArray;
class HelpEvent;

class SVX_DLLPUBLIC FmFormPage :
#if SUPD < 396
    public VCDrawPage
#else
    public SdrPage
#endif
{
    friend class FmFormObj;
    FmFormPageImpl*     m_pImpl;
    String              m_sPageName;
    StarBASIC*          m_pBasic;

public:
    TYPEINFO();

    FmFormPage(FmFormModel& rModel,StarBASIC*, FASTBOOL bMasterPage=sal_False);
    FmFormPage(const FmFormPage& rPage);
    ~FmFormPage();

    virtual void    SetModel(SdrModel* pNewModel);

    virtual SdrPage* Clone() const;
    using SdrPage::Clone;

    virtual void    NbcInsertObject(SdrObject* pObj, sal_uInt32 nPos=CONTAINER_APPEND,
                                    const SdrInsertReason* pReason=NULL);
    virtual void    InsertObject(SdrObject* pObj, sal_uInt32 nPos=CONTAINER_APPEND,
                                    const SdrInsertReason* pReason=NULL);

    virtual SdrObject* NbcRemoveObject(sal_uInt32 nObjNum);
    virtual SdrObject* RemoveObject(sal_uInt32 nObjNum);

    virtual SdrObject* NbcReplaceObject(SdrObject* pNewObj, sal_uInt32 nObjNum);
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, sal_uInt32 nObjNum);

    // Zugriff auf alle Formulare
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer>& GetForms( bool _bForceCreate = true ) const;

    FmFormPageImpl*  GetImpl() const {return m_pImpl;}

public:
    const String&       GetName() const { return m_sPageName; }
    void                SetName( const String& rName ) { m_sPageName = rName; }
    StarBASIC*          GetBasic() const { return m_pBasic; }
    sal_Bool            RequestHelp(
                            Window* pWin,
                            SdrView* pView,
                            const HelpEvent& rEvt );
};

#endif          // _SVX_FMPAGE_HXX

