/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptPage.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:42:53 $
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
#ifndef _REPORT_RPTUIPAGE_HXX
#define _REPORT_RPTUIPAGE_HXX

#ifndef _SVDPAGE_HXX
#include "svx/svdpage.hxx"
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCOMPONENT_HPP_
#include <com/sun/star/report/XReportComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif
#include "dllapi.h"

namespace rptui
{
//============================================================================
// OReportPage
//============================================================================

class OReportModel;
//class OReportSection;

class REPORTDESIGN_DLLPUBLIC OReportPage : public SdrPage
{
    OReportModel&   rModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                    m_xSection;

    OReportPage(const OReportPage&);

    bool m_bSpecialInsertMode;
    std::vector<SdrObject*> m_aTemporaryObjectList;
    // methode to remove temporary objects, created by 'special mode'
    // (BegDragObj)
    void removeTempObject(SdrObject *_pToRemoveObj);

protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();
public:
    TYPEINFO();

    OReportPage( OReportModel& rModel
                ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
                ,FASTBOOL bMasterPage=FALSE );

    virtual ~OReportPage();

    virtual SdrPage* Clone() const;
    using SdrPage::Clone;

    virtual void NbcInsertObject(SdrObject* pObj, ULONG nPos, const SdrInsertReason* pReason);
    virtual SdrObject* RemoveObject(ULONG nObjNum);

    /** returns teh index inside the object list which belongs to the report component.
        @param  _xObject    the report component
    */
    ULONG getIndexOf(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

    /** removes the SdrObject which belongs to the report component.
        @param  _xObject    the report component
    */
    void removeSdrObject(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

    void setSpecialMode() {m_bSpecialInsertMode = true;}
    bool getSpecialMode() {return m_bSpecialInsertMode;}
    // all temporary objects will remove and destroy
    void resetSpecialMode();

    /** insert a new SdrObject which belongs to the report component.
        @param  _xObject    the report component
    */
    void insertObject(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xObject);

    //inline OReportSection* getReportSection() { return m_pView; }

    /** sets the view to <NULL/>
    */
    //inline void clearSection() { m_pView = NULL; }

    ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getSection() const;
};
}
#endif //_REPORT_RPTUIPAGE_HXX
