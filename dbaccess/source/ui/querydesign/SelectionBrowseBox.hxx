/*************************************************************************
 *
 *  $RCSfile: SelectionBrowseBox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:06:46 $
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
#ifndef DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX
#define DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX

#ifndef _SVX_DBBROWSE_HXX
#include <svx/dbbrowse.hxx>
#endif // _SVX_DBBROWSE_HXX
#ifndef _VECTOR_
#include <vector>
#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef DBAUI_JOINEXCHANGE_HXX
#include "JoinExchange.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

namespace dbaui
{
    //==================================================================
#define SIZ_NONE        0
#define SIZ_TOP         1
#define SIZ_BOTTOM      2
#define SIZ_LEFT        4
#define SIZ_RIGHT       8

#define BROW_FIELD_ROW          0
#define BROW_COLUMNALIAS_ROW    1
#define BROW_TABLE_ROW          2
#define BROW_ORDER_ROW          3
#define BROW_VIS_ROW            4
#define BROW_FUNCTION_ROW       5
#define BROW_CRIT1_ROW          6
#define BROW_CRIT2_ROW          7
#define BROW_CRIT3_ROW          8
#define BROW_CRIT4_ROW          9
#define BROW_CRIT5_ROW          10
#define BROW_CRIT6_ROW          11
#define BROW_ROW_CNT            12
    //==================================================================
    class OQueryDesignView;
    class OSelectionBrowseBox : public DbBrowseBox
    {
        friend class OQueryDesignView;
        ::std::vector<FASTBOOL>             m_bVisibleRow;  // an Pos steht die RowId

        long                                m_nSeekRow;
        BrowserMode                         m_nMode;                        // Merken des BrowseModes
        Edit*                               m_pTextCell;
        DbCheckBoxCtrl*                     m_pVisibleCell;
        DbComboBoxCtrl*                     m_pFieldCell;
        DbListBoxCtrl*                      m_pFunctionCell,
                     *                      m_pTableCell,
                     *                      m_pOrderCell;

        sal_Int32                           m_nMaxColumns;              // maximale Anzahl der Spalten in einem Select-Statement

        String                              m_aFunctionStrings;
        sal_uInt16                          m_nVisibleCount;                // Anzahl der max sichtbaren Zeilen
        sal_Bool                            m_bOrderByUnRelated;
        sal_Bool                            m_bGroupByUnRelated;

    public:                         OSelectionBrowseBox( Window* pParent );
                                    ~OSelectionBrowseBox();

        void                        initialize();
        OTableFieldDesc*            InsertField( const OJoinExchangeData& jxdSource, long nColId=-1, sal_Bool bVis=sal_True, sal_Bool bActivate=sal_True );
        OTableFieldDesc*            InsertField( const OTableFieldDesc& rInfo, long nCol=-1, sal_Bool bVis=sal_True, sal_Bool bActivate=sal_True );
        void                        InsertColumn( OTableFieldDesc* pEntry, long& nColId );
        void                        RemoveColumn( sal_uInt16 nColId );
        void                        DeleteFields( const String& rAliasName );
        // AddGroupBy:: F"ugt ein Feld mit Funktion == Grupierung. Falls das Feld schon vorhanden ist und ein Aggregate Funktion
        // benutzt, wird das Flag nicht gesetzt
        void                        AddGroupBy( const OTableFieldDesc& rInfo );
        void                        AddCondition( const OTableFieldDesc& rInfo,
                                                  const String& rValue,
                                                  const sal_uInt16 nLevel, const char* pOp=0 );
        void                        AddOrder(const OTableFieldDesc& rInfo, const EOrderDir eDir, sal_uInt16& nPos);
        void                        ClearAll();
        OTableFieldDesc*            AppendNewCol( sal_uInt16 nCnt=1 );
        void                        Save();
        OQueryDesignView*           getDesignView();
        OQueryDesignView*           getDesignView() const;
        void                        SetColWidth();
        sal_uInt16                  FieldsCount();

        void                        SetColWidth(sal_uInt16 nColId, long lNewWidth);
                                    // beachtet im Gegensatz zum SetColumnWidth der Basisklasse auch eine eventuell aktive Zelle in dieser Spalte

        String                      GetCellContents(sal_uInt16 nCellIndex, long nColId);
        void                        SetCellContents(sal_uInt16 nCellIndex, long nColId, const String& strNewText);
                                        // Zelleninhalt (als String formatiert) setzen/liefern
        sal_Int32                   GetNoneVisibleRows() const;
        void                        SetNoneVisbleRow(long nRows);
        sal_Bool                    IsRowVisible(sal_uInt16 _nWhich) const;
        void                        SetRowVisible(sal_uInt16 _nWhich, sal_Bool _bVis);

        void                        SetReadOnly(sal_Bool bRO);

    protected:
        virtual sal_Bool            SeekRow( long nRow );

        virtual void                PaintStatusCell(OutputDevice& rDev, const Rectangle& rRect) const;
        virtual void                PaintCell(OutputDevice& rDev, const Rectangle& rRect,
                                              sal_uInt16 nColumnId ) const;

        virtual sal_Int8            AcceptDrop( const BrowserAcceptDropEvent& rEvt );
        virtual sal_Int8            ExecuteDrop( const BrowserExecuteDropEvent& rEvt );
//      virtual sal_Bool            QueryDrop(const BrowserDropEvent& rEvt);
//      virtual sal_Bool            Drop(const BrowserDropEvent& rEvt);
        virtual void                MouseButtonDown( const BrowserMouseEvent& rEvt );
        virtual void                MouseButtonUp( const BrowserMouseEvent& rEvt );
        virtual void                KeyInput( const KeyEvent& rEvt );
        virtual void                Command(const CommandEvent& rEvt);
        virtual void                ArrangeControls(sal_uInt16& nX, sal_uInt16 nY);

        virtual DbCellController*   GetController(long nRow, sal_uInt16 nCol);
        virtual void                InitController(DbCellControllerRef& rController, long nRow, sal_uInt16 nCol);
        virtual void                CellModified();
        virtual sal_Bool            SaveModified();
        virtual void                Init();
        virtual String              GetCellText(long nRow, sal_uInt16 nColId) const;
        virtual sal_uInt32          GetTotalCellWidth(long nRow, sal_uInt16 nColId) const;
        virtual void                ColumnResized( sal_uInt16 nColId );

        virtual sal_uInt32          GetTotalCellWidth(long nRow, sal_uInt16 nColId);

        virtual sal_uInt16          GetDefaultColumnWidth(const String& rName) const;

        void                        Fill();

    private:
        OTableFieldDesc*            FindFirstFreeCol(long & rCol);
            // rCol enthaelt die Nummer (in pOTableFieldDescList) der ersten Spalte, die von sich sagt, dass sie leer ist
            // wenn es keine solche gibt, ist rCol undefiniert und der Rueckgabewert NULL
        void                        CheckFreeColumns(long& rCol);
            // testet, ob es noch freie Spalten gibt, wenn nicht, wird ein neuer Packen angefuegt
            // rCol enthaelt die Nummer der ersten freien Spalte (in pOTableFieldDescList)

        void                        RemoveField( sal_uInt16 nId, sal_Bool bActivate = sal_True);
        Rectangle                   GetInvalidRect( sal_uInt16 nColId );
        long                        GetRealRow(long nRow) const;
        long                        GetBrowseRow(long nRowId) const;
        sal_Bool                    GetFunktionName(String& rFkt);
    };
}
#endif // DBAUI_QUERYDESIGN_OSELECTIONBROWSEBOX_HXX



