/*************************************************************************
 *
 *  $RCSfile: browserids.hxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-16 10:34:02 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#define DBACCESS_UI_BROWSER_ID_HXX

#ifndef _SBASLTID_HRC
#include <offmgr/sbasltid.hrc>
#endif
#ifndef _DBACCESS_SLOTID_HRC_
#include "dbaccess_slotid.hrc"
#endif

#define ID_BROWSER_COPY                     SID_COPY
#define ID_BROWSER_CUT                      SID_CUT
#define ID_BROWSER_EDITDOC                  SID_EDITDOC
#define ID_BROWSER_UNDORECORD               SID_UNDO
#define ID_BROWSER_PASTE                    SID_PASTE
#define ID_BROWSER_REDO                     SID_REDO
#define ID_BROWSER_SAVEDOC                  SID_SAVEDOC
#define ID_BROWSER_SAVEASDOC                SID_SAVEASDOC
#define ID_BROWSER_TITLE                    SID_DOCINFO_TITLE
#define ID_BROWSER_UNDO                     SID_UNDO
#define ID_BROWSER_INSERTCOLUMNS            SID_SBA_BRW_INSERT
#define ID_BROWSER_FORMLETTER               SID_SBA_BRW_MERGE
#define ID_BROWSER_INSERTCONTENT            SID_SBA_BRW_UPDATE

#define ID_BROWSER_SEARCH                   SID_FM_SEARCH
#define ID_BROWSER_SORTUP                   SID_FM_SORTUP
#define ID_BROWSER_SORTDOWN                 SID_FM_SORTDOWN
#define ID_BROWSER_AUTOFILTER               SID_FM_AUTOFILTER
#define ID_BROWSER_FILTERCRIT               SID_FM_FILTERCRIT
#define ID_BROWSER_ORDERCRIT                SID_FM_ORDERCRIT
#define ID_BROWSER_REMOVEFILTER             SID_FM_REMOVE_FILTER_SORT
#define ID_BROWSER_FILTERED                 SID_FM_FORM_FILTERED
#define ID_BROWSER_REFRESH                  SID_FM_REFRESH
#define ID_BROWSER_EXPL_PREVLEVEL           SID_EXPLORER_PREVLEVEL
#define ID_BROWSER_COLATTRSET               20  // Spaltenformatierung
#define ID_BROWSER_COLWIDTH                 21  // Spaltenbreite
#define ID_BROWSER_TABLEATTR                22  // table format attributes
#define ID_BROWSER_ROWHEIGHT                23  // Zeilenhoehe
#define ID_BROWSER_COLUMNINFO               24  // copies the column description to insert it into the table design
#define ID_BROWSER_COUNTALL                 SID_FM_COUNTALL // count all
#define ID_BROWSER_SQL                      SID_SOURCEVIEW
#define ID_BROWSER_ADDTABLE                 SID_FM_ADDTABLE
#define ID_BROWSER_DESIGN                   SID_SBA_QRY_DESIGN
#define ID_BROWSER_EXPLORER                 SID_DSBROWSER_EXPLORER
#define ID_BROWSER_DOCUMENT_DATASOURCE      SID_DOCUMENT_DATA_SOURCE

// The following ids are local to special components (e.g. menus), so they don't need to be unique
// overall. Please have this in mind when changing anything
#define ID_TREE_ADMINISTRATE                 1
#define ID_TREE_CLOSE_CONN                   2
#define ID_TREE_REBUILD_CONN                 3
#define ID_TREE_TABLE_COPY                   4
#define ID_TREE_QUERY_COPY                   5
#define ID_TREE_TABLE_PASTE                  6
#define ID_TREE_RELATION_DESIGN              7
#define ID_TABLE_DESIGN_NO_CONNECTION        8
#define ID_OPEN_DOCUMENT                     9
#define ID_EDIT_DOCUMENT                    10
#define ID_NEW_LINK                         11
#define ID_EDIT_LINK                        12
#define ID_DROP_LINK                        13
#define ID_CREATE_NEW_DOC                   14
#define ID_FORM_NEW_TEXT                    15
#define ID_FORM_NEW_CALC                    16
#define ID_FORM_NEW_IMPRESS                 17
#define ID_FORM_NEW_PILOT                   18
#define ID_FORM_NEW_TEMPLATE                19
#define ID_NEW_QUERY_DESIGN                 20
#define ID_EDIT_QUERY_DESIGN                21
#define ID_NEW_QUERY_SQL                    22
#define ID_EDIT_QUERY_SQL                   23
#define ID_DROP_QUERY                       24
#define ID_NEW_TABLE_DESIGN                 25
#define ID_EDIT_TABLE                       26
#define ID_DROP_TABLE                       27
#define ID_NEW_VIEW_DESIGN                  28
#define ID_DROP_VIEW                        29
#define ID_TREE_VIEW_COPY                   30
#define ID_TREE_VIEW_PASTE                  31
#define ID_DIRECT_SQL                       32
#define ID_TREE_QUERY_PASTE                 33
#define ID_BROWSER_REFRESH_REBUILD          34



#define ID_BROWSER_QUERY_EXECUTE            SID_FM_EXECUTE
#define ID_BROWSER_CLEAR_QUERY              SID_SBA_CLEAR_QUERY
#define ID_REALTION_ADD_RELATION            SID_SBA_ADD_RELATION
#define ID_BROWSER_QUERY_VIEW_FUNCTIONS     SID_SBA_QUERY_VIEW_FUNCTIONS
#define ID_BROWSER_QUERY_VIEW_TABLES        SID_SBA_QUERY_VIEW_TABLES
#define ID_BROWSER_QUERY_VIEW_ALIASES       SID_SBA_QUERY_VIEW_ALIASES
#define ID_BROWSER_QUERY_DISTINCT_VALUES    SID_SBA_QUERY_DISTINCT_VALUES
#define ID_BROWSER_CLOSE                    SID_CLOSEDOC
#define ID_BROWSER_ESACPEPROCESSING         SID_FM_NATIVESQL

#define ID_QUERY_FUNCTION                   (SID_SBA_START + 41) // Funktionen anzeigen
#define ID_QUERY_TABLENAME                  (SID_SBA_START + 42) // Tabellennamen anzeigen
#define ID_QUERY_ALIASNAME                  (SID_SBA_START + 43) // Aliasnamen anzeigen
#define ID_QUERY_DISTINCT                   (SID_SBA_START + 44) // Distinct anzeigen
#define ID_QUERY_ZOOM_IN                    SID_ZOOM_IN
#define ID_QUERY_ZOOM_OUT                   SID_ZOOM_OUT


#endif // DBACCESS_UI_BROWSER_ID_HXX
