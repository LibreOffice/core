/*************************************************************************
 *
 *  $RCSfile: logindialog.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-03-29 13:17:10 $
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

#ifndef __FRAMEWORK_SERVICES_LOGINDIALOG_HXX_
#define __FRAMEWORK_SERVICES_LOGINDIALOG_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_OMUTEXMEMBER_HXX_
#include <helper/omutexmember.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#include <services/logindialog.hrc>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif

#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _SV_MOREBTN_HXX
#include <vcl/morebtn.hxx>
#endif

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

#ifdef WNT
    #define ININAME                             DECLARE_ASCII("login.ini")
#elif defined UNIX
    #define ININAME                             DECLARE_ASCII("loginrc")
#else
    #error "name of login profile unknown!"
#endif

#define UNCPATHSEPERATOR                        sal_Unicode(0x002F)
#define MAX_SERVERHISTORY                       10

//  Use follow keys in follow order.
//  [Global]
//  UserName=as
//  ActiveServer=2
//  ConnectionType=compressed_secure
//  Language=en-US
//
//  [DefaultPorts]
//  plain=8081
//  secure=8082
//  compressed_secure=8083
//  compressed=8084
//
//  [ServerHistory]
//  Server_1=localhost
//  Server_2=munch:7202
//  Server_3=www.xxx.com:8000

#define SECTION_GLOBAL                          "Global"
#define SECTION_DEFAULTPORTS                    "DefaultPorts"
#define SECTION_SERVERHISTORY                   "ServerHistory"

#define SECTION_USERNAME                        SECTION_GLOBAL
#define SECTION_ACTIVESERVER                    SECTION_GLOBAL
#define SECTION_CONNECTIONTYPE                  SECTION_GLOBAL
#define SECTION_LANGUAGE                        SECTION_GLOBAL
#define SECTION_PLAIN                           SECTION_DEFAULTPORTS
#define SECTION_SECURE                          SECTION_DEFAULTPORTS
#define SECTION_COMPRESSEDSECURE                SECTION_DEFAULTPORTS
#define SECTION_COMPRESSED                      SECTION_DEFAULTPORTS
#define SECTION_SERVER_X                        SECTION_SERVERHISTORY

#define KEY_USERNAME                            "UserName"
#define KEY_ACTIVESERVER                        "ActiveServer"
#define KEY_CONNECTIONTYPE                      "ConnectionType"
#define KEY_LANGUAGE                            "Language"
#define KEY_PLAIN                               "plain"
#define KEY_SECURE                              "secure"
#define KEY_COMPRESSEDSECURE                    "compressed_secure"
#define KEY_COMPRESSED                          "compressed"
#define KEY_SERVER_X                            "Server_"

#define PROPERTYNAME_CONNECTIONTYPE             DECLARE_ASCII("ConnectionType"                  )
#define PROPERTYNAME_LANGUAGE                   DECLARE_ASCII("Language"                        )
#define PROPERTYNAME_PASSWORD                   DECLARE_ASCII("Password"                        )
#define PROPERTYNAME_SERVER                     DECLARE_ASCII("Server"                          )
#define PROPERTYNAME_SERVERHISTORY              DECLARE_ASCII("ServerHistory"                   )
#define PROPERTYNAME_USERNAME                   DECLARE_ASCII("UserName"                        )
#define PROPERTYNAME_COMPRESSED                 DECLARE_ASCII("compressed"                      )
#define PROPERTYNAME_COMPRESSEDSECURE           DECLARE_ASCII("compressed_secure"               )
#define PROPERTYNAME_PLAIN                      DECLARE_ASCII("plain"                           )
#define PROPERTYNAME_SECURE                     DECLARE_ASCII("secure"                          )

#define PROPERTYHANDLE_CONNECTIONTYPE           1
#define PROPERTYHANDLE_LANGUAGE                 2
#define PROPERTYHANDLE_PASSWORD                 3
#define PROPERTYHANDLE_SERVER                   4
#define PROPERTYHANDLE_SERVERHISTORY            5
#define PROPERTYHANDLE_USERNAME                 6
#define PROPERTYHANDLE_COMPRESSED               7
#define PROPERTYHANDLE_COMPRESSEDSECURE         8
#define PROPERTYHANDLE_PLAIN                    9
#define PROPERTYHANDLE_SECURE                   10

#define PROPERTYCOUNT                           10

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

struct tIMPL_DialogData
{
    ::rtl::OUString                         sUserName               ;
    ::rtl::OUString                         sPassword               ;
    css::uno::Sequence< ::rtl::OUString >   seqServerList           ;
    sal_Int32                               nActiveServer           ;
    ::rtl::OUString                         sConnectionType         ;
    css::lang::Locale                       aLanguage               ;
    sal_Int32                               nPortPlain              ;
    sal_Int32                               nPortSecure             ;
    sal_Int32                               nPortCompressedSecure   ;
    sal_Int32                               nPortCompressed         ;

    // default ctor to initialize empty structure.
    tIMPL_DialogData()
        :   sUserName               ( ::rtl::OUString()                                         )
        ,   sPassword               ( ::rtl::OUString()                                         )
        ,   seqServerList           ( css::uno::Sequence< ::rtl::OUString >()                   )
        ,   nActiveServer           ( 1                                                         )
        ,   sConnectionType         ( ::rtl::OUString()                                         )
        ,   aLanguage               ( ::rtl::OUString(), ::rtl::OUString(), ::rtl::OUString()   )
        ,   nPortPlain              ( 0                                                         )
        ,   nPortSecure             ( 0                                                         )
        ,   nPortCompressedSecure   ( 0                                                         )
        ,   nPortCompressed         ( 0                                                         )
    {
    }

    // copy ctor to initialize structure with values from another one.
    tIMPL_DialogData( const tIMPL_DialogData& aCopyDataSet )
        :   sUserName               ( aCopyDataSet.sUserName                )
        ,   sPassword               ( aCopyDataSet.sPassword                )
        ,   seqServerList           ( aCopyDataSet.seqServerList            )
        ,   nActiveServer           ( aCopyDataSet.nActiveServer            )
        ,   sConnectionType         ( aCopyDataSet.sConnectionType          )
        ,   aLanguage               ( aCopyDataSet.aLanguage                )
        ,   nPortPlain              ( aCopyDataSet.nPortPlain               )
        ,   nPortSecure             ( aCopyDataSet.nPortSecure              )
        ,   nPortCompressedSecure   ( aCopyDataSet.nPortCompressedSecure    )
        ,   nPortCompressed         ( aCopyDataSet.nPortCompressed          )
    {
    }

    // assignment operator to cop values from another struct to this one.
    tIMPL_DialogData& operator=( const tIMPL_DialogData& aCopyDataSet )
    {
        sUserName               = aCopyDataSet.sUserName                ;
        sPassword               = aCopyDataSet.sPassword                ;
        seqServerList           = aCopyDataSet.seqServerList            ;
        nActiveServer           = aCopyDataSet.nActiveServer            ;
        sConnectionType         = aCopyDataSet.sConnectionType          ;
        aLanguage               = aCopyDataSet.aLanguage                ;
        nPortPlain              = aCopyDataSet.nPortPlain               ;
        nPortSecure             = aCopyDataSet.nPortSecure              ;
        nPortCompressedSecure   = aCopyDataSet.nPortCompressedSecure    ;
        nPortCompressed         = aCopyDataSet.nPortCompressed          ;
        return *this;
    }
};

/*-************************************************************************************************************//**
    @short      implements an "private inline" dialog class used by follow class LoginDialog to show the dialog
    @descr      This is a VCL- modal dialog and not threadsafe! We use it as private definition in the context of login dialog only!

    @implements -

    @base       ModalDialog
*//*-*************************************************************************************************************/

class cIMPL_Dialog  :   public ModalDialog
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        /*-****************************************************************************************************//**
            @short      default ctor
            @descr      This ctor initialize the dialog, load ressources but not set values on edits or check boxes!
                        These is implemented by setValues() on the same class.
                        You must give us a language identifier to describe which ressource should be used!

            @seealso    method setValues()

            @param      "aLanguage", identifier to describe ressource language
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        cIMPL_Dialog( LanguageType aLanguage );

        /*-****************************************************************************************************//**
            @short      default dtor
            @descr      This dtor deinitialize the dialog and free all used ressources.
                        But you can't get the values of the dialog. Use getValues() to do this.

            @seealso    method getValues()

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        ~cIMPL_Dialog();

        /*-****************************************************************************************************//**
            @short      set new values on dialog to show
            @descr      Use this to initialize the dialg with new values for showing before execute.

            @seealso    method getValues()

            @param      "aDataSet"; struct of variables to set it on dialog controls
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void setValues( const tIMPL_DialogData& aDataSet );

        /*-****************************************************************************************************//**
            @short      get current values from dialog controls
            @descr      Use this if you will get all values of dialog after execute.

            @seealso    method setValues()

            @param      "aDataSet"; struct of variables filled by dialog
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        tIMPL_DialogData getValues();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      get a ressource for given id from right ressource file
            @descr      This dialog need his own ressource. We can't use the global ressource manager!
                        We must use our own.
                        You must give us the ressource language. If no right ressource could be found -
                        any  existing one is used automaticly!

            @seealso    method setValues()

            @param      "nId"       ; id to convert it in right ressource id
            @param      "aLanguage" ; type of ressource language
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        static ResId impl_getResId( sal_uInt16      nId         ,
                                    LanguageType    aLanguage   );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        FixedImage          m_imageHeader           ;
        FixedText           m_textLoginText         ;
        FixedText           m_textUserName          ;
        FixedText           m_textPassword          ;
        FixedText           m_textServer            ;
        Edit                m_editUserName          ;
        Edit                m_editPassword          ;
        ComboBox            m_comboServer           ;
        OKButton            m_buttonOK              ;
        CancelButton        m_buttonCancel          ;
        tIMPL_DialogData    m_aDataSet              ;
};

/*-************************************************************************************************************//**
    @short

    @descr      -

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XDialog

    @base       OMutexMember
                OWeakObject
*//*-*************************************************************************************************************/

class LoginDialog   :   public css::lang::XTypeProvider         ,
                        public css::lang::XServiceInfo          ,
                        public css::awt::XDialog                ,
                        public OMutexMember                     ,   // Order of baseclasses is neccessary for right initialization!
                        public ::cppu::OBroadcastHelper         ,
                        public ::cppu::OPropertySetHelper       ,
                        public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         LoginDialog( const css::uno::Reference< css::lang::XMultiServiceFactory >& sFactory );

        /*-****************************************************************************************************//**
            @short      -
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ~LoginDialog();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XDialog
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      set new title of dialog
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle ) throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      return the current title of this dialog
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ::rtl::OUString SAL_CALL getTitle() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      show the dialog and return user reaction
            @descr      If user close dialog with OK we return 1 else
                        user has cancelled this dialog and we return 0.
                        You can use this return value directly as boolean.

            @seealso    -

            @param      -
            @return     1; if closed with OK
            @return     0; if cancelled

            @onerror    We return 0(FALSE).
        *//*-*****************************************************************************************************/

        virtual sal_Int16 SAL_CALL execute() throw( css::uno::RuntimeException );

        /*-****************************************************************************************************//**
            @short      not implemented yet!
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL endExecute() throw( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

        //---------------------------------------------------------------------------
        //  OPropertySetHelper
        //---------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      try to convert a property value
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!
                        You must try to convert the value of given propertyhandle and
                        return results of this operation. This will be use to ask vetoable
                        listener. If no listener have a veto, we will change value realy!
                        ( in method setFastPropertyValue_NoBroadcast(...) )

            @seealso    class OPropertySetHelper
            @seealso    method setFastPropertyValue_NoBroadcast()
            @seealso    method impl_tryToChangeProperty()

            @param      "aConvertedValue"   new converted value of property
            @param      "aOldValue"         old value of property
            @param      "nHandle"           handle of property
            @param      "aValue"            new value of property
            @return     sal_True if value will be changed, sal_FALSE otherway

            @onerror    IllegalArgumentException, if you call this with an invalid argument
        *//*-*****************************************************************************************************/

        virtual sal_Bool SAL_CALL convertFastPropertyValue(         css::uno::Any&      aConvertedValue ,
                                                                      css::uno::Any&        aOldValue       ,
                                                                    sal_Int32           nHandle         ,
                                                            const   css::uno::Any&      aValue          ) throw( css::lang::IllegalArgumentException );

        /*-****************************************************************************************************//**
            @short      set value of a transient property
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!
                        Handle and value are valid everyway! You must set the new value only.
                        After this, baseclass send messages to all listener automaticly.

            @seealso    OPropertySetHelper

            @param      "nHandle"   handle of property to change
            @param      "aValue"    new value of property
            @return     -

            @onerror    An exception is thrown.
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(         sal_Int32       nHandle ,
                                                                  const css::uno::Any&  aValue  ) throw( css::uno::Exception );

        /*-****************************************************************************************************//**
            @short      get value of a transient property
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      "nHandle"   handle of property to change
            @param      "aValue"    current value of property
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL getFastPropertyValue( css::uno::Any&  aValue  ,
                                                      sal_Int32     nHandle ) const;

        /*-****************************************************************************************************//**
            @short      return structure and information about transient properties
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      -
            @return     structure with property-informations

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        /*-****************************************************************************************************//**
            @short      return propertysetinfo
            @descr      You can call this method to get information about transient properties
                        of this object.

            @seealso    OPropertySetHelper
            @seealso    XPropertySet
            @seealso    XMultiPropertySet

            @param      -
            @return     reference to object with information [XPropertySetInfo]

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return table of all supported properties
            @descr      We need this table to initialize our helper baseclass OPropertySetHelper

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        const css::uno::Sequence< css::beans::Property > impl_getStaticPropertyDescriptor();

        /*-****************************************************************************************************//**
            @short      helper method to check if a property will change his value
            @descr      Is neccessary for vetoable listener mechanism of OPropertySethelper.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool impl_tryToChangeProperty(  const   ::rtl::OUString&                        sProperty       ,
                                            const   css::uno::Any&                          aValue          ,
                                                    css::uno::Any&                          aOldValue       ,
                                                    css::uno::Any&                          aConvertedValue ) throw( css::lang::IllegalArgumentException );

        sal_Bool impl_tryToChangeProperty(  const   css::uno::Sequence< ::rtl::OUString >&  seqProperty     ,
                                            const   css::uno::Any&                          aValue          ,
                                                    css::uno::Any&                          aOldValue       ,
                                                    css::uno::Any&                          aConvertedValue ) throw( css::lang::IllegalArgumentException );

        sal_Bool impl_tryToChangeProperty(  const   sal_Int32&                              nProperty       ,
                                            const   css::uno::Any&                          aValue          ,
                                                    css::uno::Any&                          aOldValue       ,
                                                    css::uno::Any&                          aConvertedValue ) throw( css::lang::IllegalArgumentException );

        sal_Bool impl_tryToChangeProperty(  const   css::lang::Locale&                      aProperty       ,
                                            const   css::uno::Any&                          aValue          ,
                                                    css::uno::Any&                          aOldValue       ,
                                                    css::uno::Any&                          aConvertedValue ) throw( css::lang::IllegalArgumentException );

        /*-****************************************************************************************************//**
            @short      search and open profile
            @descr      This method search and open the ini file. It initialize some member too.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_openProfile();

        /*-****************************************************************************************************//**
            @short      close profile and free some member
            @descr      This method close current opened ini file and deinitialize some member too.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_closeProfile();

        /*-****************************************************************************************************//**
            @short      check current server history
            @descr      Our current server history implementation can handle 10 elements as maximum.
                        If more then 10 elements exist; old ones will be deleted.

            @seealso    -

            @param      "seqHistory"; current history
            @return     Sequence< OUString >; checked and repaired history

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_addServerToHistory(           css::uno::Sequence< ::rtl::OUString >&  seqHistory      ,
                                                sal_Int32&                              nActiveServer   ,
                                        const   ::rtl::OUString&                        sServer         );

        /*-****************************************************************************************************//**
            @short      helper methods to read/write  properties from/to ini file
            @descr      Using of Config-Class isn't easy everytime :-(
                        Thats the reason for these helper. State of operation isn't realy important ..
                        but we assert impossible cases or occured errors!

            @seealso    -

            @param      -
            @return     -

            @onerror    Assertions are shown.
        *//*-*****************************************************************************************************/

        void                                    impl_writeUserName              (   const   ::rtl::OUString&                        sUserName       );
        void                                    impl_writeActiveServer          (           sal_Int32                               nActiveServer   );
        void                                    impl_writeServerHistory         (   const   css::uno::Sequence< ::rtl::OUString >&  lHistory        );
        void                                    impl_writeConnectionType        (   const   ::rtl::OUString&                        sConnectionType );
        void                                    impl_writeLanguage              (   const   css::lang::Locale&                      aLanguage       );
        void                                    impl_writePortPlain             (           sal_Int32                               nPort           );
        void                                    impl_writePortSecure            (           sal_Int32                               nPort           );
        void                                    impl_writePortCompressedSecure  (           sal_Int32                               nPort           );
        void                                    impl_writePortCompressed        (           sal_Int32                               nPort           );

        ::rtl::OUString                         impl_readUserName               (                                                                   );
        sal_Int32                               impl_readActiveServer           (                                                                   );
        css::uno::Sequence< ::rtl::OUString >   impl_readServerHistory          (                                                                   );
        ::rtl::OUString                         impl_readConnectionType         (                                                                   );
        css::lang::Locale                       impl_readLanguage               (                                                                   );
        sal_Int32                               impl_readPortPlain              (                                                                   );
        sal_Int32                               impl_readPortSecure             (                                                                   );
        sal_Int32                               impl_readPortCompressedSecure   (                                                                   );
        sal_Int32                               impl_readPortCompressed         (                                                                   );

    //-------------------------------------------------------------------------------------------------------------
    //  debug methods
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      debug-method to check incoming parameter of some other mehods of this class
            @descr      The following methods are used to check parameters for other methods
                        of this class. The return value is used directly for an ASSERT(...).

            @seealso    ASSERTs in implementation!

            @param      references to checking variables
            @return     sal_False on invalid parameter<BR>
                        sal_True  otherway

            @onerror    -
        *//*-*****************************************************************************************************/

    #ifdef ENABLE_ASSERTIONS

    private:

        static sal_Bool impldbg_checkParameter_LoginDialog  (   const   css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory    );
        static sal_Bool impldbg_checkParameter_setTitle     (   const   ::rtl::OUString&                                        sTitle      );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >      m_xFactory          ;   /// reference to factory, which has created this instance
        ::rtl::OUString                                             m_sININame          ;   /// full qualified path to profile UNC-notation
        Config*                                                     m_pINIManager       ;   /// manager for full access to ini file
        sal_Bool                                                    m_bInExecuteMode    ;   /// protection against setting of properties during showing of dialog
        cIMPL_Dialog*                                               m_pDialog           ;   /// VCL dialog
        tIMPL_DialogData                                            m_aPropertySet      ;

};      //  class LoginDialog

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_LOGINDIALOG_HXX_
