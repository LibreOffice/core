/*************************************************************************
 *
 *  $RCSfile: moduleoptions.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:10:39 $
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

#include "moduleoptions.hxx"

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

#ifdef css
    #error  "Who defined css? I use it as namespace value ...!"
#else
    #define css     ::com::sun::star
#endif

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          These values are used to define neccessary keys from our configuration management to support
                    all functionality of these implementation.
                    It's a fast way to make changes if some keys change his name or location!

                    Property handle are neccessary to specify right position in return list of configuration
                    for asked values. We ask it with a list of properties to get his values. The returned list
                    has the same order like our given name list!
                    e.g.:
                            NAMELIST[ PROPERTYHANDLE_xxx ] => VALUELIST[ PROPERTYHANDLE_xxx ]
*//*-*************************************************************************************************************/
#define ROOTNODE_FACTORIES                  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/Office/Factories"        ))
#define PATHSEPERATOR                       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"                             ))

// Attention: The property "ooSetupFactoryEmptyDocumentURL" is read from configuration but not used! There is
//            special code that uses hard coded strings to return them.
#define PROPERTYNAME_SHORTNAME              ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryShortName"       ))
#define PROPERTYNAME_TEMPLATEFILE           ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryTemplateFile"    ))
#define PROPERTYNAME_WINDOWATTRIBUTES       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryWindowAttributes"))
#define PROPERTYNAME_EMPTYDOCUMENTURL       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryEmptyDocumentURL"))
#define PROPERTYNAME_ICON                   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryIcon"            ))

#define PROPERTYHANDLE_SHORTNAME            0
#define PROPERTYHANDLE_TEMPLATEFILE         1
#define PROPERTYHANDLE_WINDOWATTRIBUTES     2
#define PROPERTYHANDLE_EMPTYDOCUMENTURL     3
#define PROPERTYHANDLE_ICON                 4

#define PROPERTYCOUNT                       5

#define FACTORYNAME_WRITER                  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextDocument"                ))
#define FACTORYNAME_WRITERWEB               ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.WebDocument"                 ))
#define FACTORYNAME_WRITERGLOBAL            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.GlobalDocument"              ))
#define FACTORYNAME_CALC                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.SpreadsheetDocument"        ))
#define FACTORYNAME_DRAW                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.DrawingDocument"          ))
#define FACTORYNAME_IMPRESS                 ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"))
#define FACTORYNAME_MATH                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.formula.FormulaProperties"        ))
#define FACTORYNAME_CHART                   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.chart.ChartDocument"              ))

#define FACTORYCOUNT                        8

/*-************************************************************************************************************//**
    @descr  This struct hold information about one factory. We declare a complete array which can hold infos
            for all well known factories. Values of enum "EFactory" (see header!) are directly used as index!
            So we can support a fast access on these informations.
*//*-*************************************************************************************************************/
struct FactoryInfo
{
    public:
        //---------------------------------------------------------------------------------------------------------
        // initialize empty struct
        FactoryInfo()
        {
            free();
        }

        //---------------------------------------------------------------------------------------------------------
        // easy way to reset struct member!
        void free()
        {
            bInstalled                  = sal_False         ;
            sFactory                    = ::rtl::OUString() ;
            sShortName                  = ::rtl::OUString() ;
            sTemplateFile               = ::rtl::OUString() ;
            sWindowAttributes           = ::rtl::OUString() ;
            sEmptyDocumentURL           = ::rtl::OUString() ;
            nIcon                       = 0                 ;
            bChangedTemplateFile        = sal_False         ;
            bChangedWindowAttributes    = sal_False         ;
            bChangedEmptyDocumentURL    = sal_False         ;
            bChangedIcon                = sal_False         ;
        }

        //---------------------------------------------------------------------------------------------------------
        // returns list of properties, which has changed only!
        // We use given value of sNodeBase to build full qualified pathes ...
        // Last sign of it must be "/". Beacuse we use it directly, without any additional things!
        css::uno::Sequence< css::beans::PropertyValue > getChangedProperties( const ::rtl::OUString& sNodeBase )
        {
            // a) reserve memory for max. count of changed properties
            // b) add names and values of changed ones only and count it
            // c) resize return list by using count
            css::uno::Sequence< css::beans::PropertyValue > lProperties   ( 4 );
            sal_Int8                                        nRealyChanged = 0  ;

            if( bChangedTemplateFile == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_TEMPLATEFILE;
                lProperties[nRealyChanged].Value <<= sTemplateFile;
                ++nRealyChanged;
            }
            if( bChangedWindowAttributes == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_WINDOWATTRIBUTES;
                lProperties[nRealyChanged].Value <<= sWindowAttributes;
                ++nRealyChanged;
            }
            if( bChangedEmptyDocumentURL == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_EMPTYDOCUMENTURL;
                lProperties[nRealyChanged].Value <<= sEmptyDocumentURL;
                ++nRealyChanged;
            }
            if( bChangedIcon == sal_True )
            {
                lProperties[nRealyChanged].Name    = sNodeBase + PROPERTYNAME_ICON;
                lProperties[nRealyChanged].Value <<= nIcon;
                ++nRealyChanged;
            }

            // Don't forget to reset changed flags! Otherwise we save it again and again and ...
            bChangedTemplateFile        = sal_False         ;
            bChangedWindowAttributes    = sal_False         ;
            bChangedEmptyDocumentURL    = sal_False         ;
            bChangedIcon                = sal_False         ;

            lProperties.realloc( nRealyChanged );
            return lProperties;
        }

        //---------------------------------------------------------------------------------------------------------
        // We must support setting AND marking of changed values.
        // That's why we can't make our member public. We must use get/set/init methods
        // to control access on it!
        sal_Bool            getInstalled        () const { return bInstalled;         };
        ::rtl::OUString     getFactory          () const { return sFactory;           };
        ::rtl::OUString     getShortName        () const { return sShortName;         };
        ::rtl::OUString     getTemplateFile     () const { return sTemplateFile;      };
        ::rtl::OUString     getWindowAttributes () const { return sWindowAttributes;  };
        ::rtl::OUString     getEmptyDocumentURL () const { return sEmptyDocumentURL;  };
        sal_Int32           getIcon             () const { return nIcon;              };

        //---------------------------------------------------------------------------------------------------------
        // If you call set-methods - we check for changes of valkues and mark it.
        // But if you whish to set it without that ... you must initialize it!
        void initInstalled        ( sal_Bool               bNewInstalled        ) { bInstalled        = bNewInstalled        ; }
        void initFactory          ( const ::rtl::OUString& sNewFactory          ) { sFactory          = sNewFactory          ; }
        void initShortName        ( const ::rtl::OUString& sNewShortName        ) { sShortName        = sNewShortName        ; }
        void initTemplateFile     ( const ::rtl::OUString& sNewTemplateFile     ) { sTemplateFile     = sNewTemplateFile     ; }
        void initWindowAttributes ( const ::rtl::OUString& sNewWindowAttributes ) { sWindowAttributes = sNewWindowAttributes ; }
        void initEmptyDocumentURL ( const ::rtl::OUString& sNewEmptyDocumentURL ) { sEmptyDocumentURL = sNewEmptyDocumentURL ; }
        void initIcon             ( sal_Int32              nNewIcon             ) { nIcon             = nNewIcon             ; }

        //---------------------------------------------------------------------------------------------------------
        void setInstalled( sal_Bool bNewInstalled )
        {
            bInstalled = bNewInstalled;
        };

        //---------------------------------------------------------------------------------------------------------
        void setFactory( const ::rtl::OUString& sNewFactory )
        {
            sFactory = sNewFactory;
        };

        //---------------------------------------------------------------------------------------------------------
        void setShortName( const ::rtl::OUString& sNewShortName )
        {
            sShortName = sNewShortName;
        };

        //---------------------------------------------------------------------------------------------------------
        void setTemplateFile( const ::rtl::OUString& sNewTemplateFile )
        {
            if( sTemplateFile != sNewTemplateFile )
            {
                sTemplateFile        = sNewTemplateFile;
                bChangedTemplateFile = sal_True        ;
            }
        };

        //---------------------------------------------------------------------------------------------------------
        void setWindowAttributes( const ::rtl::OUString& sNewWindowAttributes )
        {
            if( sWindowAttributes != sNewWindowAttributes )
            {
                sWindowAttributes        = sNewWindowAttributes;
                bChangedWindowAttributes = sal_True            ;
            }
        };

        //---------------------------------------------------------------------------------------------------------
        void setEmptyDocumentURL( const ::rtl::OUString& sNewEmptyDocumentURL )
        {
            if( sEmptyDocumentURL != sNewEmptyDocumentURL )
            {
                sEmptyDocumentURL        = sNewEmptyDocumentURL;
                bChangedEmptyDocumentURL = sal_True            ;
            }
        };

        //---------------------------------------------------------------------------------------------------------
        void setIcon( sal_Int32 nNewIcon )
        {
            if( nNewIcon != nNewIcon )
            {
                nNewIcon     = nNewIcon;
                bChangedIcon = sal_True;
            }
        };

    private:
        sal_Bool            bInstalled                      ;
        ::rtl::OUString     sFactory                        ;
        ::rtl::OUString     sShortName                      ;
        ::rtl::OUString     sTemplateFile                   ;
        ::rtl::OUString     sWindowAttributes               ;
        ::rtl::OUString     sEmptyDocumentURL               ;
        sal_Int32           nIcon                           ;

        sal_Bool            bChangedTemplateFile        :1  ;
        sal_Bool            bChangedWindowAttributes    :1  ;
        sal_Bool            bChangedEmptyDocumentURL    :1  ;
        sal_Bool            bChangedIcon                :1  ;
};

typedef FactoryInfo   FactoryInfoList[FACTORYCOUNT];

/*-************************************************************************************************************//**
    @short          IMPL data container for wrapper class SvtModulOptions!
    @descr          These class is used as a static data container of class SvtModuleOptions. The hold it by using
                    a refcount and make it threadsafe by using an osl mutex. So we don't must do anything for that.
                    We can implement pure functionality to read/write configuration data only.

    @implements     -
    @base           ConfigItem

    @devstatus      ready to use
    @threadsafe     no
*//*-*************************************************************************************************************/
class SvtModuleOptions_Impl : public ::utl::ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------
    public:
        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
         SvtModuleOptions_Impl();
        ~SvtModuleOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------
        virtual void Notify( const css::uno::Sequence< ::rtl::OUString >& lPropertyNames );
        virtual void Commit(                                                             );

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------
        sal_Bool        IsModuleInstalled         (       SvtModuleOptions::EModule     eModule    ) const;
        ::com::sun::star::uno::Sequence < ::rtl::OUString > GetAllServiceNames();
        ::rtl::OUString GetFactoryName            (       SvtModuleOptions::EFactory    eFactory   ) const;
        ::rtl::OUString GetFactoryShortName       (       SvtModuleOptions::EFactory    eFactory   ) const;
        ::rtl::OUString GetFactoryStandardTemplate(       SvtModuleOptions::EFactory    eFactory   ) const;
        ::rtl::OUString GetFactoryWindowAttributes(       SvtModuleOptions::EFactory    eFactory   ) const;
        ::rtl::OUString GetFactoryEmptyDocumentURL(       SvtModuleOptions::EFactory    eFactory   ) const;
        sal_Int32       GetFactoryIcon            (       SvtModuleOptions::EFactory    eFactory   ) const;
        static sal_Bool ClassifyFactoryByName     ( const ::rtl::OUString&              sName      ,
                                                          SvtModuleOptions::EFactory&   eFactory   );
        void            SetFactoryStandardTemplate(       SvtModuleOptions::EFactory    eFactory   ,
                                                    const ::rtl::OUString&              sTemplate  );
        void            SetFactoryWindowAttributes(       SvtModuleOptions::EFactory    eFactory   ,
                                                    const ::rtl::OUString&              sAttributes);

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        static css::uno::Sequence< ::rtl::OUString > impl_ExpandSetNames ( const css::uno::Sequence< ::rtl::OUString >& lSetNames   );
               void                                  impl_Read           ( const css::uno::Sequence< ::rtl::OUString >& lSetNames   );

    //-------------------------------------------------------------------------------------------------------------
    //  private types
    //-------------------------------------------------------------------------------------------------------------
    private:

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------
    private:
        FactoryInfoList     m_lFactories;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short      default ctor
    @descr      We open our configuration here and read all neccessary values from it.
                These values are cached till everyone call Commit(). Then we write changed ones back to cfg.

    @seealso    baseclass ConfigItem
    @seealso    method impl_Read()

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
SvtModuleOptions_Impl::SvtModuleOptions_Impl()
    :   ::utl::ConfigItem( ROOTNODE_FACTORIES )
{
    // First initialize list of factory infos! Otherwise we couldnt gurantee right working of these class.
    for( sal_Int32 nFactory=0; nFactory<FACTORYCOUNT; ++nFactory )
    {
        m_lFactories[nFactory].free();
    }

    // Get name list of all existing set node names in configuration to read her properties in impl_Read().
    // These list is a list of long names of our factories.
    const css::uno::Sequence< ::rtl::OUString > lFactories = GetNodeNames( ::rtl::OUString() );
    impl_Read( lFactories );

    // Enable notification for changes by using configuration directly.
    // So we can update our internal values immediatly.
    EnableNotification( lFactories );
}

/*-************************************************************************************************************//**
    @short      default dtor
    @descr      If any values of our cache was modified we should write it back to configuration.

    @attention  Don't forget to call "SetModified()" method of base class ConfigItem if any interface method
                of this class modify internal member list m_lFactories! Otherwise Commit() will never be called!!!

    @seealso    baseclass ConfigItem

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
SvtModuleOptions_Impl::~SvtModuleOptions_Impl()
{
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

/*-************************************************************************************************************//**
    @short      called for notify of configmanager
    @descr      These method is called from the ConfigManager before application ends or from the
                PropertyChangeListener if the sub tree broadcasts changes. You must update our
                internal values.

    @attention  We are registered for pure set node names only. So we can use our internal method "impl_Read()" to
                update our info list. Because - these method expand given name list to full qualified property list
                and use it to read the values. These values are filled into our internal member list m_lFactories
                at right position.

    @seealso    method impl_Read()

    @param      "lNames" is the list of set node entries which should be updated.
    @return     -

    @onerror    -
    @threadsafe no
*//*-*************************************************************************************************************/
void SvtModuleOptions_Impl::Notify( const css::uno::Sequence< ::rtl::OUString >& lNames )
{
    OSL_ENSURE( sal_False, "SvtModuleOptions_Impl::Notify()\nNot implemented yet!\n" );
}

/*-****************************************************************************************************//**
    @short      write changes to configuration
    @descr      These method writes the changed values into the sub tree
                and should always called in our destructor to guarantee consistency of config data.

    @attention  We clear complete set in configuration first and write it completly new! So we don't must
                distinguish between existing, added or removed elements. Our internal cached values
                are the only and right ones.

    @seealso    baseclass ConfigItem

    @param      -
    @return     -

    @onerror    -
    @threadsafe no
*//*-*****************************************************************************************************/
void SvtModuleOptions_Impl::Commit()
{
    // Reserve memory for ALL possible factory properties!
    // Step over all factories and get her realy changed values only.
    // Build list of these ones and use it for commit.
    css::uno::Sequence< css::beans::PropertyValue > lCommitProperties( FACTORYCOUNT*PROPERTYCOUNT );
    FactoryInfo*                                    pInfo            = NULL                        ;
    sal_Int32                                       nRealCount       = 0                           ;
    ::rtl::OUString                                 sBasePath                                      ;
    for( sal_Int32 nFactory=0; nFactory<FACTORYCOUNT; ++nFactory )
    {
        pInfo = &(m_lFactories[nFactory]);

        // These path is used to build full qualified property names ....
        // See pInfo->getChangedProperties() for further informations
        sBasePath  = PATHSEPERATOR + pInfo->getFactory() + PATHSEPERATOR;

        css::uno::Sequence< css::beans::PropertyValue > lChangedProperties = pInfo->getChangedProperties ( sBasePath );
        sal_Int32                                       nPropertyCount     = lChangedProperties.getLength();
        for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
        {
            lCommitProperties[nRealCount] = lChangedProperties[nProperty];
            ++nRealCount;
        }
    }
    // Resize commit list to real size.
    // If nothing to do - suppress calling of configuration ...
    // It could be to expensive :-)
    if( nRealCount > 0 )
    {
        lCommitProperties.realloc( nRealCount );
        SetSetProperties( ::rtl::OUString(), lCommitProperties );
    }
}

/*-****************************************************************************************************//**
    @short      access method to get internal values
    @descr      These methods implement easy access to our internal values.
                You give us right enum value to specify which module interest you ... we return right information.

    @attention  Some poeple use any value as enum ... but we support in header specified values only!
                We use it directly as index in our internal list. If enum value isn't right - we crash with an
                "index out of range"!!! Please use me right - otherwise there is no guarantee.

    @seealso    -

    @param      "eModule"  , index in list - specify module
    @param      "eFactory" , index in list - specify factory
    @param      "sTemplate", set new standard template for these factory
    @return     Queried information.

    @onerror    We return default values. (mostly "not installed"!)
    @threadsafe no
*//*-*****************************************************************************************************/
sal_Bool SvtModuleOptions_Impl::IsModuleInstalled( SvtModuleOptions::EModule eModule ) const
{
    sal_Bool bInstalled = sal_False;
    switch( eModule )
    {

        case SvtModuleOptions::E_SWRITER    :   {
            // Module writer knows more then one factory!
            if (( m_lFactories[SvtModuleOptions::E_WRITER].getInstalled() == sal_True ) ||
                ( m_lFactories[SvtModuleOptions::E_WRITERWEB].getInstalled() == sal_True ) ||
                ( m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getInstalled() == sal_True ) )
                bInstalled = sal_True;
                                                }
                                                break;
        case SvtModuleOptions::E_SCALC      :   bInstalled = m_lFactories[SvtModuleOptions::E_CALC].getInstalled();
                                                break;
        case SvtModuleOptions::E_SDRAW      :   bInstalled = m_lFactories[SvtModuleOptions::E_DRAW].getInstalled();
                                                break;
        case SvtModuleOptions::E_SIMPRESS   :   bInstalled = m_lFactories[SvtModuleOptions::E_IMPRESS].getInstalled();
                                                break;
        case SvtModuleOptions::E_SMATH      :   bInstalled = m_lFactories[SvtModuleOptions::E_MATH].getInstalled();
                                                break;
        case SvtModuleOptions::E_SCHART     :   bInstalled = m_lFactories[SvtModuleOptions::E_CHART].getInstalled();
                                                break;
        case SvtModuleOptions::E_SBASIC     :   bInstalled = sal_True; // Couldn't be deselected by setup yet!
                                                break;
    }

    return bInstalled;
}

::com::sun::star::uno::Sequence < ::rtl::OUString > SvtModuleOptions_Impl::GetAllServiceNames()
{
    sal_uInt32 nCount=0;
    if( m_lFactories[SvtModuleOptions::E_WRITER].getInstalled() )
        nCount++;
    if ( m_lFactories[SvtModuleOptions::E_WRITERWEB].getInstalled() )
        nCount++;
    if ( m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SCALC].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SDRAW].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SIMPRESS].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SCHART].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SMATH].getInstalled() )
        nCount++;
    if( m_lFactories[SvtModuleOptions::E_SBASIC].getInstalled() )
        nCount++;

    css::uno::Sequence < ::rtl::OUString > aRet( nCount );
    sal_Int32 n=0;
    if( m_lFactories[SvtModuleOptions::E_WRITER].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_WRITER].getFactory();
    if ( m_lFactories[SvtModuleOptions::E_WRITERWEB].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_WRITERWEB].getFactory();
    if ( m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_WRITERGLOBAL].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SCALC].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SCALC].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SDRAW].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SDRAW].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SIMPRESS].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SIMPRESS].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SCHART].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SCHART].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SMATH].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SMATH].getFactory();
    if( m_lFactories[SvtModuleOptions::E_SBASIC].getInstalled() )
        aRet[n++] = m_lFactories[SvtModuleOptions::E_SBASIC].getFactory();

    return aRet;
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions_Impl::GetFactoryName( SvtModuleOptions::EFactory eFactory ) const
{
    ::rtl::OUString sName;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        sName = m_lFactories[eFactory].getFactory();
    }

    return sName;
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions_Impl::GetFactoryShortName( SvtModuleOptions::EFactory eFactory ) const
{
    // Attention: Hard configured yet ... because it's not fine to make changes possible by xml file yet.
    //            But it's good to plan further possibilities!

    //return m_lFactories[eFactory].sShortName;

    ::rtl::OUString sShortName;
    switch( eFactory )
    {
        case SvtModuleOptions::E_WRITER        :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter"));
                                                  break;
        case SvtModuleOptions::E_WRITERWEB     :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web"));
                                                  break;
        case SvtModuleOptions::E_WRITERGLOBAL  :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument"));
                                                  break;
        case SvtModuleOptions::E_CALC          :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("scalc"));
                                                  break;
        case SvtModuleOptions::E_DRAW          :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw"));
                                                  break;
        case SvtModuleOptions::E_IMPRESS       :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("simpress"));
                                                  break;
        case SvtModuleOptions::E_MATH          :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("smath"));
                                                  break;
        case SvtModuleOptions::E_CHART         :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("schart"));
                                                  break;
        case SvtModuleOptions::E_BASIC         :  sShortName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sbasic"));
                                                  break;
    }

    return sShortName;
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions_Impl::GetFactoryStandardTemplate( SvtModuleOptions::EFactory eFactory ) const
{
    ::rtl::OUString sFile;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        sFile = m_lFactories[eFactory].getTemplateFile();
    }

    return sFile;
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions_Impl::GetFactoryWindowAttributes( SvtModuleOptions::EFactory eFactory ) const
{
    ::rtl::OUString sAttributes;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        sAttributes = m_lFactories[eFactory].getWindowAttributes();
    }

    return sAttributes;
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions_Impl::GetFactoryEmptyDocumentURL( SvtModuleOptions::EFactory eFactory ) const
{
    // Attention: Hard configured yet ... because it's not fine to make changes possible by xml file yet.
    //            But it's good to plan further possibilities!

    //return m_lFactories[eFactory].getEmptyDocumentURL();

    ::rtl::OUString sURL;
    switch( eFactory )
    {
        case SvtModuleOptions::E_WRITER        :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/swriter"));
                                                  break;
        case SvtModuleOptions::E_WRITERWEB     :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/swriter/web"));
                                                  break;
        case SvtModuleOptions::E_WRITERGLOBAL  :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/swriter/GlobalDocument"));
                                                  break;
        case SvtModuleOptions::E_CALC          :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/scalc"));
                                                  break;
        case SvtModuleOptions::E_DRAW          :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/sdraw"));
                                                  break;
        case SvtModuleOptions::E_IMPRESS       :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/simpress?slot=10425"));
                                                  break;
        case SvtModuleOptions::E_MATH          :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/smath"));
                                                  break;
        case SvtModuleOptions::E_CHART         :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/schart"));
                                                  break;
        case SvtModuleOptions::E_BASIC         :  sURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/sbasic"));
                                                  break;
    }
    return sURL;
}

//*****************************************************************************************************************
sal_Int32 SvtModuleOptions_Impl::GetFactoryIcon( SvtModuleOptions::EFactory eFactory ) const
{
    sal_Int32 nIcon = 0;

    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        nIcon = m_lFactories[eFactory].getIcon();
    }

    return nIcon;
}

//*****************************************************************************************************************
void SvtModuleOptions_Impl::SetFactoryStandardTemplate(       SvtModuleOptions::EFactory eFactory   ,
                                                        const ::rtl::OUString&           sTemplate  )
{
    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        m_lFactories[eFactory].setTemplateFile( sTemplate );
        SetModified();
    }
}

//*****************************************************************************************************************
void SvtModuleOptions_Impl::SetFactoryWindowAttributes(       SvtModuleOptions::EFactory eFactory   ,
                                                        const ::rtl::OUString&           sAttributes)
{
    if( eFactory>=0 && eFactory<FACTORYCOUNT )
    {
        m_lFactories[eFactory].setWindowAttributes( sAttributes );
        SetModified();
    }
}

/*-************************************************************************************************************//**
    @short      return list of key names of ouer configuration management which represent our module tree
    @descr      You give use a list of current existing set node names .. and we expand it for all
                well known properties which are neccessary for this implementation.
                These full expanded list should be used to get values of this properties.

    @seealso    ctor

    @param      -
    @return     List of all relative addressed properties of given set entry names.

    @onerror    List will be empty.
    @threadsafe no
*//*-*************************************************************************************************************/
css::uno::Sequence< ::rtl::OUString > SvtModuleOptions_Impl::impl_ExpandSetNames( const css::uno::Sequence< ::rtl::OUString >& lSetNames )
{
    sal_Int32                             nCount     = lSetNames.getLength() ;
    css::uno::Sequence< ::rtl::OUString > lPropNames ( nCount*PROPERTYCOUNT );
    sal_Int32                             nPropStart = 0                     ;

    for( sal_Int32 nName=0; nName<nCount; ++nName )
    {
        lPropNames[nPropStart+PROPERTYHANDLE_SHORTNAME       ] = lSetNames[nName] + PATHSEPERATOR + PROPERTYNAME_SHORTNAME       ;
        lPropNames[nPropStart+PROPERTYHANDLE_TEMPLATEFILE    ] = lSetNames[nName] + PATHSEPERATOR + PROPERTYNAME_TEMPLATEFILE    ;
        lPropNames[nPropStart+PROPERTYHANDLE_WINDOWATTRIBUTES] = lSetNames[nName] + PATHSEPERATOR + PROPERTYNAME_WINDOWATTRIBUTES;
        lPropNames[nPropStart+PROPERTYHANDLE_EMPTYDOCUMENTURL] = lSetNames[nName] + PATHSEPERATOR + PROPERTYNAME_EMPTYDOCUMENTURL;
        lPropNames[nPropStart+PROPERTYHANDLE_ICON            ] = lSetNames[nName] + PATHSEPERATOR + PROPERTYNAME_ICON            ;
        nPropStart += PROPERTYCOUNT;
    }

    return lPropNames;
}

/*-************************************************************************************************************//**
    @short      helper to classify given factory by name
    @descr      Every factory has his own long and short name. So we can match right enum value for internal using.

    @attention  We change in/out parameter "eFactory" in every case! But you should use it only, if return value is TRUE!
                Algorithm:  Set out-parameter to propably value ... and check the longname.
                            If it match with these factory - break operation and return true AND right set parameter.
                            Otherwise try next one and so on. If no factory was found return false. Out parameter eFactory
                            is set to last tried value but shouldn't be used! Because our return value is false!

    @seealso    -

    @param      "sLongName" , long name of factory, which should be classified
    @return     "eFactory"  , right enum value, which match given long name
                and true for successfully classification, false otherwise

    @onerror    We return false.
    @threadsafe no
*//*-*************************************************************************************************************/
sal_Bool SvtModuleOptions_Impl::ClassifyFactoryByName( const ::rtl::OUString& sName, SvtModuleOptions::EFactory& eFactory )
{
    sal_Bool bState;

    eFactory = SvtModuleOptions::E_WRITER     ;
    bState   = ( sName == FACTORYNAME_WRITER );

    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_WRITERWEB     ;
        bState   = ( sName == FACTORYNAME_WRITERWEB );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_WRITERGLOBAL     ;
        bState   = ( sName == FACTORYNAME_WRITERGLOBAL );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_CALC     ;
        bState   = ( sName == FACTORYNAME_CALC );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_DRAW     ;
        bState   = ( sName == FACTORYNAME_DRAW );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_IMPRESS     ;
        bState   = ( sName == FACTORYNAME_IMPRESS );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_MATH     ;
        bState   = ( sName == FACTORYNAME_MATH );
    }
    // no else!
    if( bState == sal_False )
    {
        eFactory = SvtModuleOptions::E_CHART     ;
        bState   = ( sName == FACTORYNAME_CHART );
    }

    return bState;
}

/*-************************************************************************************************************//**
    @short      read factory configuration
    @descr      Give us a list of pure factory names (long names!) which can be used as
                direct set node names ... and we read her property values and fill internal list.
                These method can be used by initial reading at ctor and later updating by "Notify()".

    @seealso    ctor
    @seealso    method Notify()

    @param      "lFactories" is the list of set node entries which should be readed.
    @return     -

    @onerror    We do nothing.
    @threadsafe no
*//*-*************************************************************************************************************/
void SvtModuleOptions_Impl::impl_Read( const css::uno::Sequence< ::rtl::OUString >& lFactories )
{
    // Expand every set node name in lFactories to full qualified pathes to his properties
    // and get right values from configuration.
    const css::uno::Sequence< ::rtl::OUString > lProperties = impl_ExpandSetNames( lFactories  );
    const css::uno::Sequence< css::uno::Any >   lValues     = GetProperties      ( lProperties );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    OSL_ENSURE( !(lProperties.getLength()!=lValues.getLength()), "SvtModuleOptions_Impl::impl_Read()\nI miss some values of configuration keys!\n" );

    // Algorithm:   We step over all given factory names and classify it. These enum value can be used as direct index
    //              in our member list m_lFactories! VAriable nPropertyStart marks start position of every factory
    //              and her properties in expanded property/value list. The defines PROPERTHANDLE_xxx are used as offset values
    //              added to nPropertyStart. So we can address every property relative in these lists.
    //              If we found any valid values ... we reset all existing informations for corresponding m_lFactories-entry and
    //              use a pointer to these struct in memory directly to set new values.
    //              But we set it only, if bInstalled is true. Otherwise all other values of a factory can be undeclared .. They
    //              shouldn't be used then.
    // Attention:   If a propertyset of a factory will be ignored we must step to next start position of next factory infos!
    //              see "nPropertyStart += PROPERTYCOUNT" ...

    sal_Int32                   nPropertyStart  = 0                     ;
    sal_Int32                   nNodeCount      = lFactories.getLength();
    FactoryInfo*                pInfo           = NULL                  ;
    SvtModuleOptions::EFactory  eFactory                                ;
    ::rtl::OUString             sFactoryName                            ;
    ::rtl::OUString             sTemp                                   ;
    sal_Int32                   nTemp                                   ;
    for( sal_Int32 nSetNode=0; nSetNode<nNodeCount; ++nSetNode )
    {
        sFactoryName = lFactories[nSetNode];
        if( ClassifyFactoryByName( sFactoryName, eFactory ) == sal_True )
        {
            pInfo = &(m_lFactories[eFactory]);
            pInfo->free();

            pInfo->initInstalled( sal_True     );
            pInfo->initFactory  ( sFactoryName );
            lValues[nPropertyStart+PROPERTYHANDLE_SHORTNAME       ] >>= sTemp;
            pInfo->initShortName( sTemp );
            lValues[nPropertyStart+PROPERTYHANDLE_TEMPLATEFILE    ] >>= sTemp;
            pInfo->initTemplateFile( sTemp );
            lValues[nPropertyStart+PROPERTYHANDLE_WINDOWATTRIBUTES] >>= sTemp;
            pInfo->initWindowAttributes( sTemp );
            lValues[nPropertyStart+PROPERTYHANDLE_EMPTYDOCUMENTURL] >>= sTemp;
            pInfo->initEmptyDocumentURL( sTemp );
            lValues[nPropertyStart+PROPERTYHANDLE_ICON            ] >>= nTemp;
            pInfo->initIcon( nTemp );
        }
        nPropertyStart += PROPERTYCOUNT;
    }
}


//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtModuleOptions_Impl*  SvtModuleOptions::m_pDataContainer  = NULL  ;
sal_Int32               SvtModuleOptions::m_nRefCount       = 0     ;

/*-************************************************************************************************************//**
    @short      standard constructor and destructor
    @descr      This will initialize an instance with default values. We initialize/deinitialize our static data
                container and create a static mutex, which is used for threadsafe code in further time of this object.

    @seealso    method impl_GetOwnStaticMutex()

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
SvtModuleOptions::SvtModuleOptions()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    ++m_nRefCount;
    if( m_nRefCount == 1 )
    {
        m_pDataContainer = new SvtModuleOptions_Impl;
    }
}

//*****************************************************************************************************************
SvtModuleOptions::~SvtModuleOptions()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    --m_nRefCount;
    if( m_nRefCount == 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

/*-************************************************************************************************************//**
    @short      access to configuration data
    @descr      This methods allow read/write access to configuration values.
                They are threadsafe. All calls are forwarded to impl-data-container. See there for further informations!

    @seealso    method impl_GetOwnStaticMutex()

    @param      -
    @return     -

    @onerror    -
    @threadsafe yes
*//*-*************************************************************************************************************/
sal_Bool SvtModuleOptions::IsModuleInstalled( EModule eModule ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( eModule );
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions::GetFactoryName( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryName( eFactory );
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions::GetFactoryShortName( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryShortName( eFactory );
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions::GetFactoryStandardTemplate( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryStandardTemplate( eFactory );
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions::GetFactoryWindowAttributes( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryWindowAttributes( eFactory );
}

//*****************************************************************************************************************
::rtl::OUString SvtModuleOptions::GetFactoryEmptyDocumentURL( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryEmptyDocumentURL( eFactory );
}

//*****************************************************************************************************************
sal_Int32 SvtModuleOptions::GetFactoryIcon( EFactory eFactory ) const
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetFactoryIcon( eFactory );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::ClassifyFactoryByName( const ::rtl::OUString& sName    ,
                                                        EFactory&        eFactory )
{
    // We don't need any mutex here ... because we don't use any member here!
    return SvtModuleOptions_Impl::ClassifyFactoryByName( sName, eFactory );
}

//*****************************************************************************************************************
void SvtModuleOptions::SetFactoryStandardTemplate(       EFactory         eFactory   ,
                                                   const ::rtl::OUString& sTemplate  )
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->SetFactoryStandardTemplate( eFactory, sTemplate );
}

//*****************************************************************************************************************
void SvtModuleOptions::SetFactoryWindowAttributes(       EFactory         eFactory   ,
                                                   const ::rtl::OUString& sAttributes)
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->SetFactoryWindowAttributes( eFactory, sAttributes );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsMath( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsMath()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SMATH );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsChart( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsChart()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SCHART );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsCalc( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsCalc()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SCALC );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsDraw( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsDraw()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SDRAW );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsWriter( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsWriter()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SWRITER );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsImpress( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsImpress()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsModuleInstalled( E_SIMPRESS );
}

//*****************************************************************************************************************
sal_Bool SvtModuleOptions::IsBasicIDE( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::IsBasicIDE()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    return sal_True;
}

//*****************************************************************************************************************
sal_uInt32 SvtModuleOptions::GetFeatures( sal_Bool bClient ) const
{
    OSL_ENSURE( !(bClient==sal_True), "SvtModuleOptions::GetFeatures()\nWho use special parameter [bClient=TRUE]? It's obsolete!" );
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );

    sal_uInt32 nFeature = 0;

    if( m_pDataContainer->IsModuleInstalled( E_SWRITER ) == sal_True )
        nFeature |= FEATUREFLAG_WRITER;
    if( m_pDataContainer->IsModuleInstalled( E_SCALC ) == sal_True )
        nFeature |= FEATUREFLAG_CALC;
    if( m_pDataContainer->IsModuleInstalled( E_SDRAW ) == sal_True )
        nFeature |= FEATUREFLAG_DRAW;
    if( m_pDataContainer->IsModuleInstalled( E_SIMPRESS ) == sal_True )
        nFeature |= FEATUREFLAG_IMPRESS;
    if( m_pDataContainer->IsModuleInstalled( E_SCHART ) == sal_True )
        nFeature |= FEATUREFLAG_CHART;
    if( m_pDataContainer->IsModuleInstalled( E_SMATH ) == sal_True )
        nFeature |= FEATUREFLAG_MATH;
    if( m_pDataContainer->IsModuleInstalled( E_SBASIC ) == sal_True )
        nFeature |= FEATUREFLAG_BASICIDE;

    return nFeature;
}

/*-****************************************************************************************************//**
    @short      return a reference to a static mutex
    @descr      These class is threadsafe.
                We create a static mutex only for one time and use it to protect our refcount and container
                member!

    @seealso    -

    @param      -
    @return     A reference to a static mutex member.

    @onerror    -
    @threadsafe yes
*//*-*****************************************************************************************************/
::osl::Mutex& SvtModuleOptions::impl_GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static ::osl::Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static ::osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

::rtl::OUString SvtModuleOptions::GetModuleName( EModule eModule ) const
{
    switch( eModule )
    {
        case SvtModuleOptions::E_SWRITER    :   { return ::rtl::OUString::createFromAscii("Writer"); break; }
        case SvtModuleOptions::E_SCALC      :   { return ::rtl::OUString::createFromAscii("Calc"); break; }
        case SvtModuleOptions::E_SDRAW      :   { return ::rtl::OUString::createFromAscii("Draw"); break; }
        case SvtModuleOptions::E_SIMPRESS   :   { return ::rtl::OUString::createFromAscii("Impress"); break; }
        case SvtModuleOptions::E_SMATH      :   { return ::rtl::OUString::createFromAscii("Math"); break; }
        case SvtModuleOptions::E_SCHART     :   { return ::rtl::OUString::createFromAscii("Chart"); break; }
        case SvtModuleOptions::E_SBASIC     :   { return ::rtl::OUString::createFromAscii("Basic"); break; }
    }

    return ::rtl::OUString();
}

::rtl::OUString SvtModuleOptions::GetModuleName( EFactory eFactory ) const
{
    switch( eFactory )
    {
        case SvtModuleOptions::E_WRITER         :   { return ::rtl::OUString::createFromAscii("Writer"); break; }
        case SvtModuleOptions::E_WRITERWEB      :   { return ::rtl::OUString::createFromAscii("Writer"); break; }
        case SvtModuleOptions::E_WRITERGLOBAL   :   { return ::rtl::OUString::createFromAscii("Writer"); break; }
        case SvtModuleOptions::E_CALC           :   { return ::rtl::OUString::createFromAscii("Calc"); break; }
        case SvtModuleOptions::E_DRAW           :   { return ::rtl::OUString::createFromAscii("Draw"); break; }
        case SvtModuleOptions::E_IMPRESS        :   { return ::rtl::OUString::createFromAscii("Impress"); break; }
        case SvtModuleOptions::E_MATH           :   { return ::rtl::OUString::createFromAscii("Math"); break; }
        case SvtModuleOptions::E_CHART          :   { return ::rtl::OUString::createFromAscii("Chart"); break; }
        case SvtModuleOptions::E_BASIC          :   { return ::rtl::OUString::createFromAscii("Basic"); break; }
    }

    return ::rtl::OUString();
}

::com::sun::star::uno::Sequence < ::rtl::OUString > SvtModuleOptions::GetAllServiceNames()
{
    ::osl::MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->GetAllServiceNames();
}

::rtl::OUString SvtModuleOptions::GetDefaultModuleName()
{
    ::rtl::OUString aModule;
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SWRITER))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_WRITER);
    else
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SCALC))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_CALC);
    else
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SDRAW))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_DRAW);
    else
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SIMPRESS))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_IMPRESS);
    else
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SMATH))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_MATH);
    else
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SWRITER))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_WRITERGLOBAL);
    else
    if (m_pDataContainer->IsModuleInstalled(SvtModuleOptions::E_SWRITER))
        aModule = m_pDataContainer->GetFactoryShortName(SvtModuleOptions::E_WRITERWEB);
    return aModule;
}
