/*************************************************************************
 *
 *  $RCSfile: browserids.hxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:55:46 $
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
#include <svx/svxids.hrc>
#endif
#ifndef _DBACCESS_SLOTID_HRC_
#include "dbaccess_slotid.hrc"
#endif

#define ID_BROWSER_COPY                     SID_COPY
#define ID_BROWSER_CUT                      SID_CUT
#define ID_BROWSER_EDITDOC                  SID_EDITDOC
#define ID_BROWSER_UNDORECORD               SID_FM_RECORD_UNDO
#define ID_BROWSER_SAVERECORD               SID_FM_RECORD_SAVE
#define ID_BROWSER_PASTE                    SID_PASTE
#define ID_BROWSER_CLIPBOARD_FORMAT_ITEMS   SID_CLIPBOARD_FORMAT_ITEMS
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
#define ID_BROWSER_ADDTABLE                 SID_FM_ADDTABLE
#define ID_BROWSER_DESIGN                   SID_SBA_QRY_DESIGN
#define ID_BROWSER_EXPLORER                 SID_DSBROWSER_EXPLORER
#define ID_BROWSER_DOCUMENT_DATASOURCE      SID_DOCUMENT_DATA_SOURCE

// The following ids are local to special components (e.g. menus), so they don't need to be unique
// overall. Please have this in mind when changing anything
#define ID_TREE_ADMINISTRATE                 1
#define ID_TREE_CLOSE_CONN                   2
#define ID_TREE_REBUILD_CONN                 3
#define ID_TREE_RELATION_DESIGN              7
#define ID_TABLE_DESIGN_NO_CONNECTION        8
#define ID_OPEN_DOCUMENT                     9
#define ID_EDIT_DOCUMENT                    10
// free
// free
// free
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
#define ID_DIRECT_SQL                       32
#define ID_BROWSER_REFRESH_REBUILD          34
#define ID_RENAME_ENTRY                     35
#define ID_INDEX_NEW                        36
#define ID_INDEX_DROP                       37
#define ID_INDEX_RENAME                     38
#define ID_INDEX_SAVE                       39
#define ID_INDEX_RESET                      40
#define ID_DOCUMENT_CREATE_REPWIZ           41
#define ID_BROWSER_SQL                      42
#define ID_APP_NEW_FORM                     43
#define ID_APP_NEW_QUERY_AUTO_PILOT         44
#define ID_NEW_TABLE_DESIGN_AUTO_PILOT      45
#define ID_NEW_VIEW_DESIGN_AUTO_PILOT       46
#define ID_NEW_VIEW_SQL                     47
#define ID_DOCUMENT_CREATE_REPWIZ_PRE_SEL   48
#define ID_FORM_NEW_PILOT_PRE_SEL           49
#define ID_APP_NEW_FOLDER                   50


// image ids
#define SID_DB_FORM_DELETE                  65
#define SID_DB_FORM_RENAME                  66
#define SID_DB_FORM_EDIT                    67
#define SID_DB_FORM_OPEN                    68
#define SID_DB_REPORT_DELETE                69
#define SID_DB_REPORT_RENAME                70
#define SID_DB_REPORT_EDIT                  71
#define SID_DB_REPORT_OPEN                  72
#define SID_DB_QUERY_OPEN                   73
#define SID_DB_TABLE_OPEN                   74
#define SID_DB_QUERY_EDIT                   75

// other
#define ID_BROWSER_QUERY_EXECUTE            SID_FM_EXECUTE
#define ID_BROWSER_CLEAR_QUERY              SID_SBA_CLEAR_QUERY
#define ID_RELATION_ADD_RELATION            SID_SBA_ADD_RELATION
#define ID_BROWSER_QUERY_VIEW_FUNCTIONS     SID_SBA_QUERY_VIEW_FUNCTIONS
#define ID_BROWSER_QUERY_VIEW_TABLES        SID_SBA_QUERY_VIEW_TABLES
#define ID_BROWSER_QUERY_VIEW_ALIASES       SID_SBA_QUERY_VIEW_ALIASES
#define ID_BROWSER_QUERY_DISTINCT_VALUES    SID_SBA_QUERY_DISTINCT_VALUES
#define ID_BROWSER_CLOSE                    SID_CLOSEWIN
#define ID_BROWSER_ESACPEPROCESSING         SID_FM_NATIVESQL

#define ID_QUERY_FUNCTION                   (SID_SBA_START + 41) // Funktionen anzeigen
#define ID_QUERY_TABLENAME                  (SID_SBA_START + 42) // Tabellennamen anzeigen
#define ID_QUERY_ALIASNAME                  (SID_SBA_START + 43) // Aliasnamen anzeigen
#define ID_QUERY_DISTINCT                   (SID_SBA_START + 44) // Distinct anzeigen
#define ID_QUERY_EDIT_JOINCONNECTION        (SID_SBA_START + 45) // show-edit Join
#define ID_QUERY_ZOOM_IN                    SID_ZOOM_IN
#define ID_QUERY_ZOOM_OUT                   SID_ZOOM_OUT


#endif // DBACCESS_UI_BROWSER_ID_HXX
