/*************************************************************************
 *
 *  $RCSfile: dynamicmenuoptions.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pb $ $Date: 2001-06-27 08:23:03 $
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

#pragma hdrstop

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "dynamicmenuoptions.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::std                   ;
using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::beans ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_MENUS                                  OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Menus/"     ))
#define PATHDELIMITER                                   OUString(RTL_CONSTASCII_USTRINGPARAM("/"                        ))

#define SETNODE_NEWMENU                                 OUString(RTL_CONSTASCII_USTRINGPARAM("New"                      ))
#define SETNODE_WIZARDMENU                              OUString(RTL_CONSTASCII_USTRINGPARAM("Wizard"                   ))
#define SETNODE_HELPBOOKMARKS                           OUString(RTL_CONSTASCII_USTRINGPARAM("HelpBookmarks"            ))

#define PROPERTYNAME_URL                                DYNAMICMENU_PROPERTYNAME_URL
#define PROPERTYNAME_TITLE                              DYNAMICMENU_PROPERTYNAME_TITLE
#define PROPERTYNAME_IMAGEIDENTIFIER                    DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER
#define PROPERTYNAME_TARGETNAME                         DYNAMICMENU_PROPERTYNAME_TARGETNAME

#define PROPERTYCOUNT                                   4

#define OFFSET_URL                                      0
#define OFFSET_TITLE                                    1
#define OFFSET_IMAGEIDENTIFIER                          2
#define OFFSET_TARGETNAME                               3

#define PATHPREFIX                                      OUString(RTL_CONSTASCII_USTRINGPARAM("m"                        ))

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

struct MenuItem
{
    MenuItem()
    {
    }

    MenuItem(  const   OUString&   sNewURL              ,
               const   OUString&   sNewTitle            ,
               const   OUString&   sNewImageIdentifier  ,
               const   OUString&   sNewTargetName       )
    {
        sURL                = sNewURL               ;
        sTitle              = sNewTitle             ;
        sImageIdentifier    = sNewImageIdentifier   ;
        sTargetName         = sNewTargetName        ;
    }

    OUString    sURL                ;
    OUString    sTitle              ;
    OUString    sImageIdentifier    ;
    OUString    sTargetName         ;
};

class SvtDynamicMenuOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtDynamicMenuOptions_Impl();
        ~SvtDynamicMenuOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& lPropertyNames );

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit();

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void                                    Clear       (           EDynamicMenuType    eMenu           );
        Sequence< Sequence< PropertyValue > >   GetMenu     (           EDynamicMenuType    eMenu           ) const ;
        void                                    AppendItem  (           EDynamicMenuType    eMenu           ,
                                                                const   OUString&           sURL            ,
                                                                const   OUString&           sTitle          ,
                                                                const   OUString&           sImageIdentifier,
                                                                const   OUString&           sTargetName     );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management and support dynamical menu item lists!

            @seealso    -

            @param      "nNewCount"     ,   returns count of menu entries for "new"
            @param      "nWizardCount"  ,   returns count of menu entries for "wizard"
            @return     A list of configuration key names is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< OUString > impl_GetPropertyNames( sal_uInt32& nNewCount, sal_uInt32& nWizardCount, sal_uInt32& nHelpBookmarksCount );

        /*-****************************************************************************************************//**
            @short      convert routine
            @descr      Intern we hold ouer values in a deque. Sometimes we need his content as a return luence.
                        Then we must convert ouer internal format to extern.
                        That is the reason for these method!

            @seealso    -

            @param      "aList" list in vector format.
            @return     A list which right format is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< Sequence< PropertyValue > > impl_GetSequenceFromList( const vector< MenuItem >& aList ) const ;

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        vector< MenuItem >  m_aNewMenu            ;
        vector< MenuItem >  m_aWizardMenu         ;
        vector< MenuItem >  m_aHelpBookmarksMenu  ;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtDynamicMenuOptions_Impl::SvtDynamicMenuOptions_Impl()
    // Init baseclasses first
    :   ConfigItem( ROOTNODE_MENUS )
    // Init member then...
{
    // Use our list snapshot of configuration keys to get his values.
    // See impl_GetPropertyNames() for further informations.
    sal_uInt32              nNewCount          = 0;
    sal_uInt32              nWizardCount       = 0;
    sal_uInt32              nHelpBookmarksCount= 0;
    Sequence< OUString >    lNames    = impl_GetPropertyNames ( nNewCount, nWizardCount, nHelpBookmarksCount );
    Sequence< Any >         lValues   = GetProperties         ( lNames                  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtDynamicMenuOptions_Impl::SvtDynamicMenuOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    // Attention: List for names and values have an internal construction pattern!
    //
    // first "New" menu ...
    //      Name                            Value
    //      /New/1/URL                      "private:factory/swriter"
    //      /New/1/Title                    "Neues Writer Dokument"
    //      /New/1/ImageIdentifier          "icon_writer"
    //      /New/1/TargetName               "_blank"
    //
    //      /New/2/URL                      "private:factory/scalc"
    //      /New/2/Title                    "Neues Calc Dokument"
    //      /New/2/ImageIdentifier          "icon_calc"
    //      /New/2/TargetName               "_blank"
    //
    // second "Wizard" menu ...
    //      /Wizard/1/URL                   "file://b"
    //      /Wizard/1/Title                 "MalWas"
    //      /Wizard/1/ImageIdentifier       "icon_?"
    //      /Wizard/1/TargetName            "_self"
    //
    //      ... and so on ...

    MenuItem    aItem         ;
    sal_uInt32  nItem     = 0 ;
    sal_uInt32  nPosition = 0 ;
    OUString    sName         ;

    // Get names/values for new menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nNewCount; ++nItem )
    {
        lValues[nPosition] >>= aItem.sURL             ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle           ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName      ;
        ++nPosition;
        m_aNewMenu.push_back( aItem );
    }

    // Attention: Don't reset nPosition here!

    // Get names/values for wizard menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nWizardCount; ++nItem )
    {
        lValues[nPosition] >>= aItem.sURL             ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle           ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName      ;
        ++nPosition;
        m_aWizardMenu.push_back( aItem );
    }

    // Attention: Don't reset nPosition here!

    // Get names/values for wizard menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nHelpBookmarksCount; ++nItem )
    {
        lValues[nPosition] >>= aItem.sURL             ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle           ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier ;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName      ;
        ++nPosition;
        m_aHelpBookmarksMenu.push_back( aItem );
    }

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    EnableNotification( lNames );
*/
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtDynamicMenuOptions_Impl::~SvtDynamicMenuOptions_Impl()
{
    // We must save our current values .. if user forget it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtDynamicMenuOptions_Impl::Notify( const Sequence< OUString >& lPropertyNames )
{
    DBG_ASSERT( sal_False, "SvtDynamicMenuOptions_Impl::Notify()\nNot implemented yet! I don't know how I can handle a dynamical list of unknown properties ...\n" );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtDynamicMenuOptions_Impl::Commit()
{
    // Write all properties!
    // Delete complete sets first.
    ClearNodeSet( SETNODE_NEWMENU    );
    ClearNodeSet( SETNODE_WIZARDMENU );
    ClearNodeSet( SETNODE_HELPBOOKMARKS );

    MenuItem                    aItem                           ;
    OUString                    sNode                           ;
    Sequence< PropertyValue >   lPropertyValues( PROPERTYCOUNT );
    sal_uInt32                  nItem          = 0              ;

    // Copy "new" menu entries to save-list!
    sal_uInt32 nNewCount = m_aNewMenu.size();
    for( nItem=0; nItem<nNewCount; ++nItem )
    {
        aItem = m_aNewMenu[nItem];
        // Format:  "New/1/URL"
        //          "New/1/Title"
        //          ...
        sNode = SETNODE_NEWMENU + PATHDELIMITER + PATHPREFIX + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;

        lPropertyValues[OFFSET_URL             ].Name  =   sNode + PROPERTYNAME_URL             ;
        lPropertyValues[OFFSET_TITLE           ].Name  =   sNode + PROPERTYNAME_TITLE           ;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Name  =   sNode + PROPERTYNAME_IMAGEIDENTIFIER ;
        lPropertyValues[OFFSET_TARGETNAME      ].Name  =   sNode + PROPERTYNAME_TARGETNAME      ;

        lPropertyValues[OFFSET_URL             ].Value <<= aItem.sURL                           ;
        lPropertyValues[OFFSET_TITLE           ].Value <<= aItem.sTitle                         ;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Value <<= aItem.sImageIdentifier               ;
        lPropertyValues[OFFSET_TARGETNAME      ].Value <<= aItem.sTargetName                    ;

        SetSetProperties( SETNODE_NEWMENU, lPropertyValues );
    }

    // Copy "wizard" menu entries to save-list!
    sal_uInt32 nWizardCount = m_aWizardMenu.size();
    for( nItem=0; nItem<nWizardCount; ++nItem )
    {
        aItem = m_aWizardMenu[nItem];
        // Format:  "Wizard/1/URL"
        //          "Wizard/1/Title"
        //          ...
        sNode = SETNODE_WIZARDMENU + PATHDELIMITER + PATHPREFIX + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;

        lPropertyValues[OFFSET_URL             ].Name  =   sNode + PROPERTYNAME_URL             ;
        lPropertyValues[OFFSET_TITLE           ].Name  =   sNode + PROPERTYNAME_TITLE           ;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Name  =   sNode + PROPERTYNAME_IMAGEIDENTIFIER ;
        lPropertyValues[OFFSET_TARGETNAME      ].Name  =   sNode + PROPERTYNAME_TARGETNAME      ;

        lPropertyValues[OFFSET_URL             ].Value <<= aItem.sURL                           ;
        lPropertyValues[OFFSET_TITLE           ].Value <<= aItem.sTitle                         ;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Value <<= aItem.sImageIdentifier               ;
        lPropertyValues[OFFSET_TARGETNAME      ].Value <<= aItem.sTargetName                    ;

        SetSetProperties( SETNODE_WIZARDMENU, lPropertyValues );
    }

    // Copy help bookmarks entries to save-list!
    sal_uInt32 nHelpBookmarksCount = m_aHelpBookmarksMenu.size();
    for( nItem=0; nItem<nHelpBookmarksCount; ++nItem )
    {
        aItem = m_aHelpBookmarksMenu[nItem];
        // Format:  "HelpBookmarks/1/URL"
        //          "HelpBookmarks/1/Title"
        //          ...
        sNode = SETNODE_HELPBOOKMARKS + PATHDELIMITER + PATHPREFIX + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;

        lPropertyValues[OFFSET_URL             ].Name  =   sNode + PROPERTYNAME_URL             ;
        lPropertyValues[OFFSET_TITLE           ].Name  =   sNode + PROPERTYNAME_TITLE           ;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Name  =   sNode + PROPERTYNAME_IMAGEIDENTIFIER ;
        lPropertyValues[OFFSET_TARGETNAME      ].Name  =   sNode + PROPERTYNAME_TARGETNAME      ;

        lPropertyValues[OFFSET_URL             ].Value <<= aItem.sURL                           ;
        lPropertyValues[OFFSET_TITLE           ].Value <<= aItem.sTitle                         ;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Value <<= aItem.sImageIdentifier               ;
        lPropertyValues[OFFSET_TARGETNAME      ].Value <<= aItem.sTargetName                    ;

        SetSetProperties( SETNODE_HELPBOOKMARKS, lPropertyValues );
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtDynamicMenuOptions_Impl::Clear( EDynamicMenuType eMenu )
{
    switch( eMenu )
    {
        case E_NEWMENU      :   {
                                    m_aNewMenu.clear();
                                    SetModified();
                                }
                                break;

        case E_WIZARDMENU   :   {
                                    m_aWizardMenu.clear();
                                    SetModified();
                                }
                                break;

        case E_HELPBOOKMARKS :  {
                                    m_aHelpBookmarksMenu.clear();
                                    SetModified();
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtDynamicMenuOptions_Impl::GetMenu( EDynamicMenuType eMenu ) const
{
    Sequence< Sequence< PropertyValue > > lReturn;
    switch( eMenu )
    {
        case E_NEWMENU      :   {
                                    lReturn = impl_GetSequenceFromList( m_aNewMenu );
                                }
                                break;

        case E_WIZARDMENU   :   {
                                    lReturn = impl_GetSequenceFromList( m_aWizardMenu );
                                }
                                break;

        case E_HELPBOOKMARKS :  {
                                    lReturn = impl_GetSequenceFromList( m_aHelpBookmarksMenu );
                                }
                                break;
    }
    return lReturn;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtDynamicMenuOptions_Impl::AppendItem(            EDynamicMenuType    eMenu           ,
                                                const   OUString&           sURL            ,
                                                const   OUString&           sTitle          ,
                                                const   OUString&           sImageIdentifier,
                                                const   OUString&           sTargetName     )
{
    MenuItem aItem( sURL, sTitle, sImageIdentifier, sTargetName );

    switch( eMenu )
    {
        case E_NEWMENU  :   {
                                m_aNewMenu.push_back( aItem );
                                SetModified();
                            }
                            break;

        case E_WIZARDMENU   :   {
                                m_aWizardMenu.push_back( aItem );
                                SetModified();
                            }
                            break;

        case E_HELPBOOKMARKS :  {
                                m_aHelpBookmarksMenu.push_back( aItem );
                                SetModified();
                            }
                            break;
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtDynamicMenuOptions_Impl::impl_GetPropertyNames( sal_uInt32& nNewCount, sal_uInt32& nWizardCount, sal_uInt32& nHelpBookmarksCount )
{
    // First get ALL names of current existing list items in configuration!
    Sequence< OUString > lNewItems           = GetNodeNames( SETNODE_NEWMENU       );
    Sequence< OUString > lWizardItems        = GetNodeNames( SETNODE_WIZARDMENU    );
    Sequence< OUString > lHelpBookmarksItems = GetNodeNames( SETNODE_HELPBOOKMARKS );

    // Get information about list counts ...
    nNewCount           = lNewItems.getLength          ();
    nWizardCount        = lWizardItems.getLength       ();
    nHelpBookmarksCount = lHelpBookmarksItems.getLength();
    // ... and create a property list with right size!
    // 4 properties for every item
    Sequence< OUString > lProperties( (nNewCount+nWizardCount+nHelpBookmarksCount)*PROPERTYCOUNT );

    sal_uInt32  nPosition = 0   ;
    sal_uInt32  nItem     = 0   ;
    OUString    sPosition       ;

    // BUT ... we must sort our name lists!
    // Because our configuration give it sorted by name ...
    // We need it sorted by number! And "m2" comes later then "m10" ... but 10 should be come later then 2!!!
    // We do it dureing creating of our return list.
    // We reserve lProperties[0...nNewCount-1] for new menu and lProperties[nNewCount...nNewCount+nWizardCount-1] for wizard menu.
    // In this ranges we can set our entries directly. Look for expandation of one entry to four properties too!!!

    // Add names for new menu to list.
    // 4 subkeys for every item!
    // nPosition is the start point of an menu item,
    // nItem an index into right list of node names!
    for( nItem=0; nItem<nNewCount; ++nItem )
    {
        sPosition  = lNewItems[nItem].copy( 1, lNewItems[nItem].getLength()-1 );
        nPosition  = sPosition.toInt32();
        nPosition *= PROPERTYCOUNT;

        lProperties[nPosition] = SETNODE_NEWMENU + PATHDELIMITER + lNewItems[nItem] + PATHDELIMITER + PROPERTYNAME_URL              ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_NEWMENU + PATHDELIMITER + lNewItems[nItem] + PATHDELIMITER + PROPERTYNAME_TITLE            ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_NEWMENU + PATHDELIMITER + lNewItems[nItem] + PATHDELIMITER + PROPERTYNAME_IMAGEIDENTIFIER  ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_NEWMENU + PATHDELIMITER + lNewItems[nItem] + PATHDELIMITER + PROPERTYNAME_TARGETNAME       ;
        ++nPosition;
    }

    // Attention: Don't reset nPosition here!

    // Add names for wizard menu to list.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nWizardCount; ++nItem )
    {
        sPosition  = lWizardItems[nItem].copy( 1, lWizardItems[nItem].getLength()-1 );
        nPosition  = sPosition.toInt32();
        nPosition *= PROPERTYCOUNT;
        nPosition += (nNewCount*PROPERTYCOUNT);

        lProperties[nPosition] = SETNODE_WIZARDMENU + PATHDELIMITER + lWizardItems[nItem] + PATHDELIMITER + PROPERTYNAME_URL            ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_WIZARDMENU + PATHDELIMITER + lWizardItems[nItem] + PATHDELIMITER + PROPERTYNAME_TITLE          ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_WIZARDMENU + PATHDELIMITER + lWizardItems[nItem] + PATHDELIMITER + PROPERTYNAME_IMAGEIDENTIFIER;
        ++nPosition;
        lProperties[nPosition] = SETNODE_WIZARDMENU + PATHDELIMITER + lWizardItems[nItem] + PATHDELIMITER + PROPERTYNAME_TARGETNAME     ;
        ++nPosition;
    }

    // Attention: Don't reset nPosition here!

    // Add names for help bookmarks to list.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nHelpBookmarksCount; ++nItem )
    {
        sPosition  = lHelpBookmarksItems[nItem].copy( 1, lHelpBookmarksItems[nItem].getLength()-1 );
        nPosition  = sPosition.toInt32();
        nPosition *= PROPERTYCOUNT;
        nPosition += ((nNewCount+nWizardCount)*PROPERTYCOUNT);

        lProperties[nPosition] = SETNODE_HELPBOOKMARKS + PATHDELIMITER + lHelpBookmarksItems[nItem] + PATHDELIMITER + PROPERTYNAME_URL            ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_HELPBOOKMARKS + PATHDELIMITER + lHelpBookmarksItems[nItem] + PATHDELIMITER + PROPERTYNAME_TITLE          ;
        ++nPosition;
        lProperties[nPosition] = SETNODE_HELPBOOKMARKS + PATHDELIMITER + lHelpBookmarksItems[nItem] + PATHDELIMITER + PROPERTYNAME_IMAGEIDENTIFIER;
        ++nPosition;
        lProperties[nPosition] = SETNODE_HELPBOOKMARKS + PATHDELIMITER + lHelpBookmarksItems[nItem] + PATHDELIMITER + PROPERTYNAME_TARGETNAME     ;
        ++nPosition;
    }

    // Return result.
    return lProperties;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtDynamicMenuOptions_Impl::impl_GetSequenceFromList( const vector< MenuItem >& aList ) const
{
    // Copy all elements from internal list to an external representation.
    // Don't change order of entries!
    // Copy it from beginning of list to the end.

    // Initialize return sequence with right size.
    sal_Int32                               nCount      = aList.size()   ;
    Sequence< Sequence< PropertyValue > >   lResult     ( nCount        );
    Sequence< PropertyValue >               lProperties ( PROPERTYCOUNT );
    // Copy items from given to return list.
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        lProperties[OFFSET_URL              ].Name  =   PROPERTYNAME_URL             ;
        lProperties[OFFSET_TITLE            ].Name  =   PROPERTYNAME_TITLE           ;
        lProperties[OFFSET_IMAGEIDENTIFIER  ].Name  =   PROPERTYNAME_IMAGEIDENTIFIER ;
        lProperties[OFFSET_TARGETNAME       ].Name  =   PROPERTYNAME_TARGETNAME      ;

        lProperties[OFFSET_URL              ].Value <<= aList[nItem].sURL            ;
        lProperties[OFFSET_TITLE            ].Value <<= aList[nItem].sTitle          ;
        lProperties[OFFSET_IMAGEIDENTIFIER  ].Value <<= aList[nItem].sImageIdentifier;
        lProperties[OFFSET_TARGETNAME       ].Value <<= aList[nItem].sTargetName     ;

        lResult[nItem] = lProperties;
    }
    return lResult;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtDynamicMenuOptions_Impl*     SvtDynamicMenuOptions::m_pDataContainer = NULL  ;
sal_Int32                       SvtDynamicMenuOptions::m_nRefCount      = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtDynamicMenuOptions::SvtDynamicMenuOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtDynamicMenuOptions_Impl;
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtDynamicMenuOptions::~SvtDynamicMenuOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtDynamicMenuOptions::Clear( EDynamicMenuType eMenu )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->Clear( eMenu );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtDynamicMenuOptions::GetMenu( EDynamicMenuType eMenu ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMenu( eMenu );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtDynamicMenuOptions::AppendItem(         EDynamicMenuType    eMenu           ,
                                        const   OUString&           sURL            ,
                                        const   OUString&           sTitle          ,
                                        const   OUString&           sImageIdentifier,
                                        const   OUString&           sTargetName     )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->AppendItem( eMenu, sURL, sTitle, sImageIdentifier, sTargetName );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtDynamicMenuOptions::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}
