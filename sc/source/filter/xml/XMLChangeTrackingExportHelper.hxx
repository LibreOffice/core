/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingExportHelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-14 07:13:41 $
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

#ifndef _SC_XMLCHANGETRACKINGEXPORTHELPER_HXX
#define _SC_XMLCHANGETRACKINGEXPORTHELPER_HXX

#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

class ScDocument;
class ScChangeAction;
class ScChangeTrack;
class ScXMLExport;
class ScBaseCell;
class ScChangeActionDel;
class ScBigRange;
class ScEditEngineTextObj;
class ScChangeActionTable;
class DateTime;

typedef std::list<ScChangeActionDel*> ScMyDeletionsList;

class ScChangeTrackingExportHelper
{
    ScXMLExport&    rExport;

    ScChangeTrack*  pChangeTrack;
    ScEditEngineTextObj* pEditTextObj;
    ScChangeActionTable* pDependings;
    rtl::OUString   sChangeIDPrefix;
    rtl::OUString   sAccepted;
    rtl::OUString   sRejected;
    rtl::OUString   sPending;
    com::sun::star::uno::Reference<com::sun::star::text::XText> xText;

    rtl::OUString GetChangeID(const sal_uInt32 nActionNumber);
    void GetAcceptanceState(const ScChangeAction* pAction);

    void WriteBigRange(const ScBigRange& rBigRange, const sal_Char *pName);
    void WriteChangeInfo(const ScChangeAction* pAction);
    void WriteGenerated(const ScChangeAction* pDependAction);
    void WriteDeleted(const ScChangeAction* pDependAction);
    void WriteDepending(const ScChangeAction* pDependAction);
    void WriteDependings(ScChangeAction* pAction);

    void WriteEmptyCell();
    void SetValueAttributes(const double& fValue, const String& sValue);
    void WriteValueCell(const ScBaseCell* pCell, const String& sValue);
    void WriteStringCell(const ScBaseCell* pCell);
    void WriteEditCell(const ScBaseCell* pCell);
    void WriteFormulaCell(const ScBaseCell* pCell, const String& sValue);
    void WriteCell(const ScBaseCell* pCell, const String& sValue);

    void WriteContentChange(ScChangeAction* pAction);
    void AddInsertionAttributes(const ScChangeAction* pAction);
    void WriteInsertion(ScChangeAction* pAction);
    void AddDeletionAttributes(const ScChangeActionDel* pAction, const ScChangeActionDel* pLastAction);
    void WriteDeletionCells(ScChangeActionDel* pAction);
    void WriteCutOffs(const ScChangeActionDel* pAction);
    void WriteDeletion(ScChangeAction* pAction);
    void WriteMovement(ScChangeAction* pAction);
    void WriteRejection(ScChangeAction* pAction);

    void CollectCellAutoStyles(const ScBaseCell* pBaseCell);
    void CollectActionAutoStyles(ScChangeAction* pAction);
    void WorkWithChangeAction(ScChangeAction* pAction);
public:
    ScChangeTrackingExportHelper(ScXMLExport& rExport);
    ~ScChangeTrackingExportHelper();

    void WriteChangeViewSettings();
    void CollectAutoStyles();
    void CollectAndWriteChanges();
};

#endif
