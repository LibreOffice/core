/*************************************************************************
 *
 *  $RCSfile: fmexch.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2001-07-30 14:26:12 $
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
#ifndef _SVX_FMEXCH_HXX
#define _SVX_FMEXCH_HXX

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _DTRANS_HXX //autogen
#include <so3/dtrans.hxx>
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

class FmFormShell;
class FmFormPage;
class SvLBoxEntry;

//========================================================================
// Exchange types
#define SVX_FM_FIELD_EXCH           String("SvxFormFieldExch",              sizeof("SvxFormFieldExch"))
#define SVX_FM_CONTROL_EXCH         String("SvxFormExplCtrlExch",           sizeof("SvxFormExplCtrlExch"))
#define SVX_FM_CONTROLS_AS_PATH     String("SvxFormControlsAsPathExchange", sizeof("SvxFormControlsAsPathExchange"))
#define SVX_FM_HIDDEN_CONTROLS      String("SvxFormHiddenControlsExchange", sizeof("SvxFormHiddenControlsExchange"))
#define SVX_FM_FILTER_FIELDS        String("SvxFilterFieldExchange",        sizeof("SvxFilterFieldExchange"))

//========================================================================
class SvTreeListBox;

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================

    typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<sal_uInt32> > FmControlPaths;

    DECLARE_STL_VECTOR( SvLBoxEntry*, ListBoxEntryArray );

    //====================================================================
    //= OLocalExchange
    //====================================================================
    class OLocalExchange : public TransferableHelper
    {
    private:
        sal_Bool            m_bDragging;

    public:
        class GrantAccess
        {
            friend class OLocalExchangeHelper;
        };

    public:
        OLocalExchange( );

        sal_Bool    isDragging() const { return m_bDragging; }
        void        startDrag( Window* pWindow, sal_Int8 nDragSourceActions, const GrantAccess& );

    protected:
        virtual void        DragFinished( sal_Int8 nDropAction );
        virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

        static  sal_Bool    implHasFormat( const DataFlavorExVector& _rFormats, sal_uInt32 _nFormatId );

    private:
        void StartDrag( Window* pWindow, sal_Int8 nDragSourceActions, sal_Int32 nDragPointer = DND_POINTER_NONE, sal_Int32 nDragImage = DND_IMAGE_NONE )
        {   // don't allow this base class method to be called from outside
            TransferableHelper::StartDrag(pWindow, nDragSourceActions, nDragPointer, nDragImage);
        }
    };

    //====================================================================
    //= OLocalExchangeHelper
    //====================================================================
    /// a helper for navigator windows (SvTreeListBox'es) which allow DnD within themself
    class OLocalExchangeHelper
    {
    protected:
        Window*             m_pDragSource;
        OLocalExchange*     m_pTransferable;

    public:
        OLocalExchangeHelper(Window* _pDragSource);
        ~OLocalExchangeHelper();

        void        prepareDrag( );
        void        startDrag( sal_Int8 nDragSourceActions );
        sal_Bool    isDragSource() const { return m_pTransferable && m_pTransferable->isDragging(); }

    protected:
        virtual OLocalExchange* createExchange() const = 0;

    protected:
        void implReset();
    };

    //====================================================================
    //= OControlExchange
    //====================================================================
    class OControlExchange : public OLocalExchange
    {
    protected:
        ListBoxEntryArray   m_aSelectedEntries;
        FmControlPaths      m_aControlPaths;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                            m_aHiddenControlModels;
        SvLBoxEntry*        m_pFocusEntry;
        FmFormShell*        m_pShell;
        FmFormPage*         m_pPage;

    public:
        OControlExchange( );
        OControlExchange( SvLBoxEntry* _pFocusEntry );

        void addSelectedEntry( SvLBoxEntry* _pEntry );
        void setFocusEntry( SvLBoxEntry* _pFocusEntry );

        void setShellAndPage( FmFormShell* _pShell, FmFormPage* _pPage ) { m_pShell = _pShell; m_pPage = _pPage; }

        void buildPathFormat(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot);
            // baut aus m_aSelectedEntries m_aControlPaths auf
            // (es wird davon ausgegangen, dass die Eintraege in m_aSelectedEntries sortiert sind in Bezug auf die Nachbar-Beziehung)


        void buildListFromPath(SvTreeListBox* pTreeBox, SvLBoxEntry* pRoot);
            // der umgekehrte Weg : wirft alles aus m_aSelectedEntries weg und baut es mittels m_aControlPaths neu auf

        void addHiddenControlsFormat(const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > > seqInterfaces);
            // fuegt ein SVX_FML_HIDDEN_CONTROLS-Format hinzu und merk sich dafuer die uebergebenen Interfaces
            // (es erfolgt KEINE Ueberpruefung, ob dadurch auch tatsaechlich nur hidden Controls bezeichnet werden, dass muss der
            // Aufrufer sicherstellen)

        static sal_uInt32       getFieldExchangeFormatId( );
        static sal_uInt32       getControlPathFormatId( );
        static sal_uInt32       getHiddenControlModelsFormatId( );

        inline static sal_Bool  hasFieldExchangeFormat( const DataFlavorExVector& _rFormats );
        inline static sal_Bool  hasControlPathFormat( const DataFlavorExVector& _rFormats );
        inline static sal_Bool  hasHiddenControlModelsFormat( const DataFlavorExVector& _rFormats );

        SvLBoxEntry*                focused() const { return m_pFocusEntry; }
        const ListBoxEntryArray&    selected() const { return m_aSelectedEntries; }
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                                    hiddenControls() const { return m_aHiddenControlModels; }

        FmFormShell*            getShell() const { return m_pShell; }
        FmFormPage*             getPage() const { return m_pPage; }

    protected:
        virtual void                AddSupportedFormats();
    };

    //====================================================================
    //= OControlExchangeHelper
    //====================================================================
    class OControlExchangeHelper : public OLocalExchangeHelper
    {
    public:
        OControlExchangeHelper(Window* _pDragSource) : OLocalExchangeHelper(_pDragSource) { }

        OControlExchange* operator->() const { return static_cast<OControlExchange*>(m_pTransferable); }

    protected:
        virtual OLocalExchange* createExchange() const;
    };

    //====================================================================
    //====================================================================
    inline sal_Bool OControlExchange::hasFieldExchangeFormat( const DataFlavorExVector& _rFormats )
    {
        return implHasFormat( _rFormats, getFieldExchangeFormatId() );
    }

    inline sal_Bool OControlExchange::hasControlPathFormat( const DataFlavorExVector& _rFormats )
    {
        return implHasFormat( _rFormats, getControlPathFormatId() );
    }

    inline sal_Bool OControlExchange::hasHiddenControlModelsFormat( const DataFlavorExVector& _rFormats )
    {
        return implHasFormat( _rFormats, getHiddenControlModelsFormatId() );
    }

//........................................................................
}   // namespace svxform
//........................................................................

#endif

