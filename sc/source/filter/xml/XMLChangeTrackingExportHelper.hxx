/*************************************************************************
 *
 *  $RCSfile: XMLChangeTrackingExportHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-01-17 09:32:58 $
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

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

class ScDocument;
class ScChangeAction;
class ScChangeTrack;
class ScXMLExport;
class ScBaseCell;

class ScChangeTrackingExportHelper
{
    ScXMLExport&    rExport;

    ScChangeTrack*  pChangeTrack;
    rtl::OUString   sTrackedChanges;
    rtl::OUString   sChangeIDPrefix;

    rtl::OUString GetChangeID(const sal_uInt32 nActionNumber);

    void WriteBigRange(const ScChangeAction* pAction);
    void WriteChangeInfo(const ScChangeAction* pAction);
    void WriteDepending(const ScChangeAction* pDependAction);
    void WriteDependings(ScChangeAction* pAction);

    void WriteEmptyCell();
    void WriteValueCell(const ScBaseCell* pCell);
    void WriteStringEditCell(const rtl::OUString& rString);
    void WriteStringCell(const ScBaseCell* pCell);
    void WriteEditCell(const ScBaseCell* pCell);
    void WriteFormulaCell(const ScBaseCell* pCell);
    void WriteCell(const ScBaseCell* pCell);

    void WriteContentChange(ScChangeAction* pAction);
    void WriteInsertion(ScChangeAction* pAction);
    void WriteDeletion(ScChangeAction* pAction);
    void WriteMovement(ScChangeAction* pAction);

    void WorkWithChangeAction(ScChangeAction* pAction);
    void StartChangeActionList();
    void EndChangeActionList();
public:
    ScChangeTrackingExportHelper(ScXMLExport& rExport);
    ~ScChangeTrackingExportHelper();

    void CollectAndWriteChanges();
};

#endif
