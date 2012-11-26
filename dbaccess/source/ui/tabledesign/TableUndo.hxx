/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef DBAUI_TABLEUNDO_HXX
#define DBAUI_TABLEUNDO_HXX

#ifndef DBAUI_GENERALUNDO_HXX
#include "GeneralUndo.hxx"
#endif
#ifndef _SV_MULTISEL_HXX
#include <tools/multisel.hxx>
#endif

#include <vector>

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif

namespace dbaui
{
    //========================================================================
    class OTableRowView;
    class OTableRow;
    class OTableDesignUndoAct : public OCommentUndoAction
    {
    protected:
        OTableRowView* m_pTabDgnCtrl;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableDesignUndoAct( OTableRowView* pOwner ,sal_uInt16 nCommentID);
        virtual ~OTableDesignUndoAct();
    };

    //========================================================================
    class OTableEditorCtrl;
    class OTableEditorUndoAct : public OTableDesignUndoAct
    {
    protected:
        OTableEditorCtrl* pTabEdCtrl;

    public:
        OTableEditorUndoAct( OTableEditorCtrl* pOwner,sal_uInt16 nCommentID );
        virtual ~OTableEditorUndoAct();
    };


    //========================================================================
    class OTableDesignCellUndoAct : public OTableDesignUndoAct
    {
    protected:
        sal_uInt16  m_nCol;
        long    m_nRow;
        ::com::sun::star::uno::Any  m_sOldText;
        ::com::sun::star::uno::Any  m_sNewText;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableDesignCellUndoAct( OTableRowView* pOwner, long nRowID, sal_uInt16 nColumn );
        virtual ~OTableDesignCellUndoAct();
    };

    class OTypeInfo;
    //========================================================================
    class OTableEditorTypeSelUndoAct : public OTableEditorUndoAct
    {
    protected:
        sal_uInt16          m_nCol;
        long            m_nRow;
        TOTypeInfoSP    m_pOldType;
        TOTypeInfoSP    m_pNewType;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorTypeSelUndoAct( OTableEditorCtrl* pOwner, long nRowID, sal_uInt16 nColumn, const TOTypeInfoSP& _pOldType );
        virtual ~OTableEditorTypeSelUndoAct();
    };

    //========================================================================
    class OTableEditorDelUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector< ::boost::shared_ptr<OTableRow> > m_aDeletedRows;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorDelUndoAct( OTableEditorCtrl* pOwner );
        virtual ~OTableEditorDelUndoAct();
    };

    //========================================================================
    class OTableEditorInsUndoAct : public OTableEditorUndoAct
    {
    protected:
        ::std::vector< ::boost::shared_ptr<OTableRow> > m_vInsertedRows;
        long                        m_nInsPos;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorInsUndoAct( OTableEditorCtrl* pOwner,
                                long nInsertPosition,
                                const ::std::vector<  ::boost::shared_ptr<OTableRow> >& _vInsertedRows);
        virtual ~OTableEditorInsUndoAct();
    };

    //========================================================================
    class OTableEditorInsNewUndoAct : public OTableEditorUndoAct
    {
    protected:
        long m_nInsPos;
        long m_nInsRows;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OTableEditorInsNewUndoAct( OTableEditorCtrl* pOwner, long nInsertPosition, long nInsertedRows );
        virtual ~OTableEditorInsNewUndoAct();
    };

    //========================================================================
    class OPrimKeyUndoAct : public OTableEditorUndoAct
    {
    protected:
        MultiSelection      m_aDelKeys,
                            m_aInsKeys;
        sal_Bool                m_bActPrimKeySet;
        OTableEditorCtrl* m_pEditorCtrl;

        virtual void    Undo();
        virtual void    Redo();
    public:
        OPrimKeyUndoAct( OTableEditorCtrl* pOwner, MultiSelection aDeletedKeys, MultiSelection aInsertedKeys );
        virtual ~OPrimKeyUndoAct();
    };
}
#endif // DBAUI_TABLEUNDO_HXX


