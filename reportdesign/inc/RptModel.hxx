#ifndef REPORT_RPTMODEL_HXX
#define REPORT_RPTMODEL_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RptModel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:18:02 $
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
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#include "dllapi.h"

class Window;
namespace dbaui
{
    class OSingleDocumentController;
}
namespace reportdesign
{
    class OReportDefinition;
}
namespace rptui
{
//============================================================================
// OReportModel
//============================================================================

class OReportPage;
class OXUndoEnvironment;

class REPORTDESIGN_DLLPUBLIC OReportModel : public SdrModel
{
    friend class OReportPage;

private:
    OXUndoEnvironment*                  m_pUndoEnv;
    ::dbaui::OSingleDocumentController* m_pController;
    ::reportdesign::OReportDefinition*  m_pReportDefinition;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

    OReportModel( const OReportModel& );
    void operator=(const OReportModel& rSrcModel);
public:
    TYPEINFO();

    OReportModel(::reportdesign::OReportDefinition* _pReportDefinition);
    virtual ~OReportModel();

    virtual void        SetChanged(sal_Bool bFlg = sal_True);
    virtual SdrPage*    AllocPage(FASTBOOL bMasterPage);
    virtual Window*     GetCurDocViewWin();
    virtual SdrPage*    RemovePage(USHORT nPgNum);
    /** returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType  GetPageNumType() const;

    OXUndoEnvironment&  GetUndoEnv();
    void                SetModified(sal_Bool _bModified);

    inline dbaui::OSingleDocumentController* getController() const { return m_pController; }
    inline void attachController( dbaui::OSingleDocumentController& _rController ) { m_pController = &_rController; }
    void detachController();

    OReportPage* createNewPage(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);

    /** returns the page which belongs to a section
    *
    * \param _xSection
    * \return The page or <NULL/> when t´no page could be found.
    */
    OReportPage* getPage(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);

    /// returns the XReportDefinition which the OReportModel belongs to
    ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >
                getReportDefinition() const;
};
}
#endif

