/*************************************************************************
 *
 *  $RCSfile: tdoc_docmgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:32:18 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

#ifndef INCLUDED_TDOC_DOCMGR_HXX
#define INCLUDED_TDOC_DOCMGR_HXX

#include <map>

#include "osl/mutex.hxx"

#include "cppuhelper/implbase1.hxx"

#include "com/sun/star/document/XEventBroadcaster.hpp"
#include "com/sun/star/document/XEventListener.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "drafts/com/sun/star/frame/XModuleManager.hpp"

namespace tdoc_ucp {

    class OfficeDocumentsEventListener
    {
    public:
        virtual void notifyDocumentOpened( const rtl::OUString & rDocId ) = 0;
        virtual void notifyDocumentClosed( const rtl::OUString & rDocId ) = 0;
    };

    //=======================================================================

    struct StorageInfo
    {
        rtl::OUString aTitle;
        com::sun::star::uno::Reference<
            com::sun::star::embed::XStorage > xStorage;
        com::sun::star::uno::Reference<
            com::sun::star::frame::XModel >   xModel;

        StorageInfo() {}; // needed for STL map only.

        StorageInfo(
            const rtl::OUString & rTitle,
            const com::sun::star::uno::Reference<
                com::sun::star::embed::XStorage > & rxStorage,
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & rxModel )
        : aTitle( rTitle ), xStorage( rxStorage ), xModel( rxModel ) {}
    };

    //=======================================================================

    struct ltref
    {
        bool operator()(
            const rtl::OUString & r1, const rtl::OUString & r2 ) const
        {
            return r1 < r2;
        }
    };

    typedef std::map< rtl::OUString, StorageInfo, ltref > DocumentList;

    //=======================================================================

    class OfficeDocumentsManager :
        public cppu::WeakImplHelper1< com::sun::star::document::XEventListener >
    {
    public:
        OfficeDocumentsManager(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory > & xSMgr,
            OfficeDocumentsEventListener * pDocEventListener );
        virtual ~OfficeDocumentsManager();

        void destroy();

        // document::XEventListener
        virtual void SAL_CALL notifyEvent(
                const com::sun::star::document::EventObject & Event )
            throw ( com::sun::star::uno::RuntimeException );

        // lang::XEventListener (base of document::XEventListener)
        virtual void SAL_CALL disposing(
                const com::sun::star::lang::EventObject & Source )
            throw ( com::sun::star::uno::RuntimeException );

        // Non-interface
        com::sun::star::uno::Reference< com::sun::star::embed::XStorage >
        queryStorage( const rtl::OUString & rDocId );

        rtl::OUString
        queryDocumentId(
            const com::sun::star::uno::Reference<
                com::sun::star::frame::XModel > & xModel );

        com::sun::star::uno::Reference< com::sun::star::frame::XModel >
        queryDocumentModel( const rtl::OUString & rDocId );

        com::sun::star::uno::Sequence< rtl::OUString >
        queryDocuments();

        rtl::OUString
        queryStorageTitle( const rtl::OUString & rDocId );

    private:
        static com::sun::star::uno::Reference<
            com::sun::star::document::XEventBroadcaster >
        createDocumentEventNotifier(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
        void buildDocumentsList();
        bool
        isOfficeDocument(
            const com::sun::star::uno::Reference<
                com::sun::star::uno::XInterface > & xDoc );

        osl::Mutex                                          m_aMtx;
        com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >    m_xSMgr;
        com::sun::star::uno::Reference<
            com::sun::star::document::XEventBroadcaster >   m_xDocEvtNotifier;
        com::sun::star::uno::Reference<
            drafts::com::sun::star::frame::XModuleManager > m_xModuleMgr;
        DocumentList                                        m_aDocs;
        OfficeDocumentsEventListener *                      m_pDocEventListener;
    };

} // namespace tdoc_ucp

#endif /* !INCLUDED_TDOC_DOCMGR_HXX */
