/*************************************************************************
 *
 *  $RCSfile: indexfieldscontrol.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-16 16:22:00 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_INDEXFIELDSCONTROL_HXX_
#define _DBAUI_INDEXFIELDSCONTROL_HXX_

#ifndef _SVX_DBBROWSE_HXX
#include <svx/dbbrowse.hxx>
#endif
#ifndef _DBAUI_INDEXCOLLECTION_HXX_
#include "indexcollection.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//......................................................................
namespace dbaui
{
//......................................................................

    //==================================================================
    // IndexFieldsControl
    //==================================================================
    class IndexFieldsControl : public DbBrowseBox
    {
    protected:
        IndexFields                 m_aSavedValue;

        IndexFields                 m_aFields;          // !! order matters !!
        ConstIndexFieldsIterator    m_aSeekRow;         // !!

        DbListBoxCtrl*              m_pSortingCell;
        DbListBoxCtrl*              m_pFieldNameCell;

        String                      m_sAscendingText;
        String                      m_sDescendingText;

        Link                        m_aModifyHdl;

    public:
        IndexFieldsControl( Window* _pParent, const ResId& _rId );
        ~IndexFieldsControl();

        void Init(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rAvailableFields);

        void initializeFrom(const IndexFields& _rFields);
        void commitTo(IndexFields& _rFields);

        sal_Bool SaveModified();

        const IndexFields&  GetSavedValue() const { return m_aSavedValue; }
        void                SaveValue() { m_aSavedValue = m_aFields; }

        void SetModifyHdl(const Link& _rHdl) { m_aModifyHdl = _rHdl; }
        Link GetModifyHdl() const { return m_aModifyHdl; }

    protected:
        // DbBrowseBox overridables
        virtual void PaintCell( OutputDevice& _rDev, const Rectangle& _rRect, sal_uInt16 _nColumnId ) const;
        virtual sal_Bool SeekRow(long nRow);
        virtual sal_uInt32 GetTotalCellWidth(long nRow, sal_uInt16 nColId);

        DbCellController*   GetController(long _nRow, sal_uInt16 _nColumnId);
        void                InitController(DbCellControllerRef&, long _nRow, sal_uInt16 _nColumnId);

    protected:
        String GetCurrentRowCellText(sal_uInt16 nColId) const;
        sal_Bool implGetFieldDesc(long _nRow, ConstIndexFieldsIterator& _rPos);

        sal_Bool isNewField() const { return GetCurRow() >= (sal_Int32)m_aFields.size(); }

        DECL_LINK( OnListEntrySelected, ListBox* );
    };

//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXFIELDSCONTROL_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 07.03.01 14:21:34  fs
 ************************************************************************/

