/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RecentlyUsedMasterPages.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:23:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_CONTROLS_RECENTLY_USED_MASTER_PAGES_HXX
#define SD_TOOLPANEL_CONTROLS_RECENTLY_USED_MASTER_PAGES_HXX

#include "tools/SdGlobalResourceContainer.hxx"
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
    : public SdGlobalResource
{
public:
    /** Return the single instance of this class.
    */
    static RecentlyUsedMasterPages& Instance (void);

    void AddEventListener (const Link& rEventListener);
    void RemoveEventListener (const Link& rEventListener);

    int GetMasterPageCount (void) const;
    MasterPageContainer::Token GetTokenForIndex (sal_uInt32 nIndex) const;

private:
    /** The single instance of this class.  It is created on demand when
        Instance() is called for the first time.
    */
    static RecentlyUsedMasterPages* mpInstance;

    ::std::vector<Link> maListeners;

    class MasterPageList;
    ::std::auto_ptr<MasterPageList> mpMasterPages;
    unsigned long int mnMaxListSize;
    ::boost::shared_ptr<MasterPageContainer> mpContainer;

    RecentlyUsedMasterPages (void);
    virtual ~RecentlyUsedMasterPages (void);

    /** Call this method after a new object has been created.
    */
    void LateInit (void);

    /// The copy constructor is not implemented.  Do not use!
    RecentlyUsedMasterPages (const RecentlyUsedMasterPages&);

    /// The assignment operator is not implemented.  Do not use!
    RecentlyUsedMasterPages& operator= (const RecentlyUsedMasterPages&);

    void SendEvent (void);
    DECL_LINK(MasterPageChangeListener, MasterPageObserverEvent*);
    DECL_LINK(MasterPageContainerChangeListener, MasterPageContainerChangeEvent*);

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
        MasterPageContainer::Token aToken,
        bool bMakePersistent = true);

    /** Load the list of recently used master pages from the registry where
        it was saved to make it persistent.
    */
    void LoadPersistentValues (void);

    /** Save the list of recently used master pages to the registry to make
        it presistent.
    */
    void SavePersistentValues (void);

    void ResolveList (void);
};



} } } // end of namespace ::sd::toolpanel::controls

#endif
