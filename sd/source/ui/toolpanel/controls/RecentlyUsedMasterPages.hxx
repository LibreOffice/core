/*************************************************************************
 *
 *  $RCSfile: RecentlyUsedMasterPages.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:47:50 $
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

#ifndef SD_TOOLPANEL_CONTROLS_RECENTLY_USED_MASTER_PAGES_HXX
#define SD_TOOLPANEL_CONTROLS_RECENTLY_USED_MASTER_PAGES_HXX

#include <osl/mutex.hxx>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <vector>
#include <tools/string.hxx>

#include "DrawDocShell.hxx"
#include "MasterPageContainer.hxx"

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

class SdPage;

namespace sd {
class MasterPageObserverEvent;
}


namespace sd { namespace toolpanel { namespace controls {

/** This singleton holds a list of the most recently used master pages.
*/
class RecentlyUsedMasterPages
{
public:
    /** Return the single instance of this class.
    */
    static RecentlyUsedMasterPages& Instance (void);

    void AddEventListener (const Link& rEventListener);
    void RemoveEventListener (const Link& rEventListener);

    int GetMasterPageCount (void) const;
    String GetURL (int nIndex) const;
    String GetMasterPageName (int nIndex) const;
    SdPage* GetSlide (int nIndex) const;
    SdPage* GetMasterPage (int nIndex) const;
    Image GetMasterPagePreview (int nIndex, int nWidth) const;

private:
    static ::osl::Mutex maMutex;

    /** The single instance of this class.  It is created on demand when
        Instance() is called for the first time.
    */
    static RecentlyUsedMasterPages* mpInstance;

    ::std::vector<Link> maListeners;

    typedef ::std::vector<MasterPageContainer::Token> MasterPageList;
    class PageByNameFinder;
    MasterPageList maMasterPages;
    unsigned long int mnMaxListSize;

    RecentlyUsedMasterPages (void);
    ~RecentlyUsedMasterPages (void);
    /** Call this method after a new object has been created.
    */
    void LateInit (void);

    /// The copy constructor is not implemented.  Do not use!
    RecentlyUsedMasterPages (const RecentlyUsedMasterPages&);

    /// The assignment operator is not implemented.  Do not use!
    RecentlyUsedMasterPages& operator= (const RecentlyUsedMasterPages&);

    void SendEvent (void);
    DECL_LINK(MasterPageChangeListener, MasterPageObserverEvent*);

    /** Add a descriptor for the specified master page to the end of the
        list of most recently used master pages.  When the page is already a
        member of that list the associated descriptor is moved to the end of
        the list to make it the most recently used entry.
        @param bMakePersistent
            When <TRUE/> is given then the new list of recently used master
            pages is written back into the configuration to make it
            persistent.  Giving <FALSE/> to ommit this is used while loading
            the persistent list from the configuration.
    */
    void AddMasterPage (
        const String& rsName,
        bool bMakePersistent = true);

    /** Load the list of recently used master pages from the registry where
        it was saved to make it persistent.
    */
    void LoadPersistentValues (void);

    /** Save the list of recently used master pages to the registry to make
        it presistent.
    */
    void SavePersistentValues (void);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
        OpenConfiguration (
            const ::rtl::OUString& rsRootName,
            bool bReadOnly);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
        GetConfigurationNode (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface>& xRoot,
          const ::rtl::OUString& sPathToNode);
};



} } } // end of namespace ::sd::toolpanel::controls

#endif
