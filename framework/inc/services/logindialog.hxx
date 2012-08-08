/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_LOGINDIALOG_LOGINDIALOG_HXX_
#define __FRAMEWORK_LOGINDIALOG_LOGINDIALOG_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

#include <services/logindialog.hrc>

#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/XFlushable.hpp>

#include <tools/config.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/propshlp.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/combobox.hxx>

#include <vcl/button.hxx>

#include <vcl/button.hxx>
#include <vcl/morebtn.hxx>

namespace framework{

#define ANY                                 ::com::sun::star::uno::Any
#define EXCEPTION                           ::com::sun::star::uno::Exception
#define ILLEGALARGUMENTEXCEPTION            ::com::sun::star::lang::IllegalArgumentException
#define IPROPERTYARRAYHELPER                ::cppu::IPropertyArrayHelper
#define OBROADCASTHELPER                    ::cppu::OBroadcastHelper
#define OPROPERTYSETHELPER                  ::cppu::OPropertySetHelper
#define OWEAKOBJECT                         ::cppu::OWeakObject
#define PROPERTY                            ::com::sun::star::beans::Property
#define REFERENCE                           ::com::sun::star::uno::Reference
#define RUNTIMEEXCEPTION                    ::com::sun::star::uno::RuntimeException
#define SEQUENCE                            ::com::sun::star::uno::Sequence
#define XDIALOG                             ::com::sun::star::awt::XDialog
#define XMULTISERVICEFACTORY                ::com::sun::star::lang::XMultiServiceFactory
#define XPROPERTYSETINFO                    ::com::sun::star::beans::XPropertySetInfo
#define XSERVICEINFO                        ::com::sun::star::lang::XServiceInfo
#define XTYPEPROVIDER                       ::com::sun::star::lang::XTypeProvider
#define PROPERTYVALUE                       ::com::sun::star::beans::PropertyValue
#define LOCALE                              ::com::sun::star::lang::Locale
#define XFLUSHABLE                          ::com::sun::star::util::XFlushable
#define XFLUSHLISTENER                      ::com::sun::star::util::XFlushListener

#ifdef WNT
    #define ININAME                             DECLARE_ASCII("login.ini")
#elif defined UNIX
    #define ININAME                             DECLARE_ASCII("loginrc")
#else
    #error "name of login profile unknown!"
#endif

#define UNCPATHSEPERATOR                        sal_Unicode(0x002F)

//  Use follow keys in follow order.
//  [Global]
//  UserName=as
//  ActiveServer=2
//  ConnectionType=https
//  Language=en-US
//  UseProxy=[browser|custom|none]
//  SecurityProxy=so-webcache:3128
//  dialog=[big|small]
//  [DefaultPorts]
//  https=8445
//  http=8090
//  [ServerHistory]
//  Server_1=localhost
//  Server_2=munch:7202
//  Server_3=www.xxx.com:8000

#define SECTION_GLOBAL                          "Global"
#define SECTION_DEFAULTPORTS                    "DefaultPorts"
#define SECTION_SERVERHISTORY                   "ServerHistory"

struct tIMPL_DialogData
{
    ::rtl::OUString         sUserName               ;
    ::rtl::OUString         sPassword               ;
    SEQUENCE< ::rtl::OUString > seqServerList       ;
    sal_Int32               nActiveServer           ;
    ::rtl::OUString         sConnectionType         ;
    LOCALE                  aLanguage               ;
    sal_Int32               nPortHttp               ;
    sal_Int32               nPortHttps              ;
    ANY                     aParentWindow           ;
    ::rtl::OUString         sSecurityProxy          ;
    ::rtl::OUString         sUseProxy               ;
    ::rtl::OUString         sDialog                 ;
    sal_Bool                bProxyChanged           ;

    // default ctor to initialize empty structure.
    tIMPL_DialogData()
        :   sUserName               ( ::rtl::OUString()                     )
        ,   sPassword               ( ::rtl::OUString()                     )
        ,   seqServerList           ( SEQUENCE< ::rtl::OUString >()         )
        ,   nActiveServer           ( 1                                     )
        ,   sConnectionType         ( ::rtl::OUString()                     )
        ,   aLanguage               ( ::rtl::OUString(), ::rtl::OUString(), ::rtl::OUString() )
        ,   nPortHttp               ( 0                                     )
        ,   nPortHttps              ( 0                                     )
        ,   aParentWindow           (                                       )
        ,   sSecurityProxy          ( ::rtl::OUString()                     )
        ,   sUseProxy               ( ::rtl::OUString()                     )
        ,   sDialog                 ( ::rtl::OUString()                     )
        ,   bProxyChanged           ( sal_False                             )
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
        ,   nPortHttp               ( aCopyDataSet.nPortHttp                )
        ,   nPortHttps              ( aCopyDataSet.nPortHttps               )
        ,   aParentWindow           ( aCopyDataSet.aParentWindow            )
        ,   sSecurityProxy          ( aCopyDataSet.sSecurityProxy           )
        ,   sUseProxy               ( aCopyDataSet.sUseProxy                )
        ,   sDialog                 ( aCopyDataSet.sDialog                  )
        ,   bProxyChanged           ( aCopyDataSet.bProxyChanged            )
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
        nPortHttp               = aCopyDataSet.nPortHttp                ;
        nPortHttps              = aCopyDataSet.nPortHttps               ;
        aParentWindow           = aCopyDataSet.aParentWindow            ;
        sSecurityProxy          = aCopyDataSet.sSecurityProxy           ;
        sUseProxy               = aCopyDataSet.sUseProxy                ;
        sDialog                 = aCopyDataSet.sDialog                  ;
        bProxyChanged           = aCopyDataSet.bProxyChanged            ;
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
            @descr      This ctor initialize the dialog, load resources but not set values on edits or check boxes!
                        These is implemented by setValues() on the same class.
                        You must give us a language identifier to describe which resource should be used!

            @seealso    method setValues()

            @param      "aLanguage" , identifier to describe resource language
            @param      "pParent"   , parent window handle for dialog! If is it NULL -> no parent exist ...
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        cIMPL_Dialog( ::com::sun::star::lang::Locale aLocale, Window* pParent );

        /*-****************************************************************************************************//**
            @short      default dtor
            @descr      This dtor deinitialize the dialog and free all used resources.
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

        /*-****************************************************************************************************/
        /* handler
        */

        DECL_LINK( ClickHdl, void* );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:
        void            setCustomSettings();

        void            showDialogExpanded();
        void            showDialogCollapsed();

        /*-****************************************************************************************************//**
            @short      get a host and port from a concated string form <host>:<port>

            @param      "aProxyHostPort" ; a string with the following format <host>:<port>
            @param      "aHost"          ; a host string
            @param      "aPort"          ; a port string
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void            getProxyHostPort( const ::rtl::OUString& aProxyHostPort, ::rtl::OUString& aHost, ::rtl::OUString& aPort );

        /*-****************************************************************************************************//**
            @short      get a resource for given id from right resource file
            @descr      This dialog need his own resource. We can't use the global resource manager!
                        We must use our own.
                        You must give us the resource language. If no right resource could be found -
                        any  existing one is used automaticly!

            @seealso    method setValues()

            @param      "nId"       ; id to convert it in right resource id
            @param      "aLanguage" ; type of resource language
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        static ResId impl_getResId( sal_uInt16      nId         ,
                                    ::com::sun::star::lang::Locale  aLocale );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        FixedImage          m_imageHeader               ;
        FixedText           m_textLoginText             ;
        FixedText           m_textUserName              ;
        Edit                m_editUserName              ;
        FixedText           m_textPassword              ;
        Edit                m_editPassword              ;
        FixedLine           m_fixedLineServer           ;
        FixedText           m_textServer                ;
        ComboBox            m_comboServer               ;
        FixedLine           m_fixedLineProxySettings    ;
        RadioButton         m_radioNoProxy              ;
        RadioButton         m_radioBrowserProxy         ;
        RadioButton         m_radioCustomProxy          ;
        FixedText           m_textSecurityProxy         ;
        FixedText           m_textSecurityProxyHost     ;
        Edit                m_editSecurityProxyHost     ;
        FixedText           m_textSecurityProxyPort     ;
        Edit                m_editSecurityProxyPort     ;
        FixedLine           m_fixedLineButtons          ;
        OKButton            m_buttonOK                  ;
        CancelButton        m_buttonCancel              ;
        PushButton          m_buttonAdditionalSettings  ;
        Size                m_expandedDialogSize        ;
        Size                m_collapsedDialogSize       ;
        Point               m_expOKButtonPos            ;
        Point               m_expCancelButtonPos        ;
        Point               m_expAdditionalButtonPos    ;
        Point               m_colOKButtonPos            ;
        Point               m_colCancelButtonPos        ;
        Point               m_colAdditionalButtonPos    ;
        ::rtl::OUString     m_colButtonAddText          ;
        ::rtl::OUString     m_expButtonAddText          ;
        tIMPL_DialogData    m_aDataSet                  ;
};

/*-************************************************************************************************************//**
    @short

    @descr      -

    @implements XInterface
                XTypeProvider
                XServiceInfo
                XDialog

    @base       ThreadHelpBase
                OWeakObject
*//*-*************************************************************************************************************/

class LoginDialog   :   public XTYPEPROVIDER                ,
                        public XSERVICEINFO                 ,
                        public XDIALOG                      ,
                        public XFLUSHABLE                   ,
                        private ThreadHelpBase              ,   // Order of baseclasses is neccessary for right initialization!
                        public OBROADCASTHELPER             ,
                        public OPROPERTYSETHELPER           ,
                        public OWEAKOBJECT
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

         LoginDialog( const REFERENCE< XMULTISERVICEFACTORY >& sFactory );

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
        //  XFlushable
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      write changed values to configuration
            @descr      Normaly the dialog returns with an OK or ERROR value. If OK occure - we flush data
                        auomaticly. But otherwise we do nothing. If user of this service wish to use property set
                        only without any UI(!) - he must call "flush()" explicitly to write data!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL flush() throw( RUNTIMEEXCEPTION );
        virtual void SAL_CALL addFlushListener( const REFERENCE< XFLUSHLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );
        virtual void SAL_CALL removeFlushListener( const REFERENCE< XFLUSHLISTENER >& xListener ) throw( RUNTIMEEXCEPTION );

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

        virtual void SAL_CALL setTitle( const ::rtl::OUString& sTitle ) throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      return the current title of this dialog
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual ::rtl::OUString SAL_CALL getTitle() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      show the dialog and return user reaction
            @descr      If user close dialog with OK we return 1 else
                        user has cancelled this dialog and we return 0.
                        You can use this return value directly as boolean.

            @seealso    -

            @param      -
            @return     1; if closed with OK
            @return     0; if cancelled

            @onerror    We return 0(sal_False).
        *//*-*****************************************************************************************************/

        virtual sal_Int16 SAL_CALL execute() throw( RUNTIMEEXCEPTION );

        /*-****************************************************************************************************//**
            @short      not implemented yet!
            @descr      -

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void SAL_CALL endExecute() throw( RUNTIMEEXCEPTION );

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

        virtual sal_Bool SAL_CALL convertFastPropertyValue(         ANY&        aConvertedValue ,
                                                                      ANY&      aOldValue       ,
                                                                    sal_Int32   nHandle         ,
                                                            const   ANY&        aValue          ) throw( ILLEGALARGUMENTEXCEPTION );

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

        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(         sal_Int32   nHandle ,
                                                                  const ANY&        aValue  ) throw( EXCEPTION );

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

        virtual void SAL_CALL getFastPropertyValue( ANY&        aValue  ,
                                                      sal_Int32 nHandle ) const;

        /*-****************************************************************************************************//**
            @short      return structure and information about transient properties
            @descr      This method is calling from helperclass "OPropertySetHelper".
                        Don't use this directly!

            @seealso    OPropertySetHelper

            @param      -
            @return     structure with property-informations

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual IPROPERTYARRAYHELPER& SAL_CALL getInfoHelper();

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

        virtual REFERENCE< XPROPERTYSETINFO > SAL_CALL getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException);

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

        const SEQUENCE< PROPERTY > impl_getStaticPropertyDescriptor();

        /*-****************************************************************************************************//**
            @short      helper method to check if a property will change his value
            @descr      Is neccessary for vetoable listener mechanism of OPropertySethelper.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool impl_tryToChangeProperty(  const   ::rtl::OUString&        sProperty       ,
                                            const   ANY&                    aValue          ,
                                                    ANY&                    aOldValue       ,
                                                    ANY&                    aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

        sal_Bool impl_tryToChangeProperty(  const   SEQUENCE< ::rtl::OUString >& seqProperty,
                                            const   ANY&                    aValue          ,
                                                    ANY&                    aOldValue       ,
                                                    ANY&                    aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

        sal_Bool impl_tryToChangeProperty(  const   sal_Int32&              nProperty       ,
                                            const   ANY&                    aValue          ,
                                                    ANY&                    aOldValue       ,
                                                    ANY&                    aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

        sal_Bool impl_tryToChangeProperty(  const   LOCALE&                 aProperty       ,
                                            const   ANY&                    aValue          ,
                                                    ANY&                    aOldValue       ,
                                                    ANY&                    aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

        sal_Bool impl_tryToChangeProperty(  const   ANY&                    aProperty       ,
                                            const   ANY&                    aValue          ,
                                                    ANY&                    aOldValue       ,
                                                    ANY&                    aConvertedValue ) throw( ILLEGALARGUMENTEXCEPTION );

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
            @short      write profile and free some member
            @descr      This method writes current settings and deinitialize some member too.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        void impl_writeProfile();

        /*-****************************************************************************************************//**
            @short      check current server history
            @descr      Our current server history implementation can handle 10 elements as maximum.
                        If more then 10 elements exist; old ones will be deleted.

            @seealso    -

            @param      "seqHistory"; current history
            @return     Sequence< OUString >; checked and repaired history

            @onerror    -
        *//*-*****************************************************************************************************/

        void impl_addServerToHistory(           SEQUENCE< ::rtl::OUString >& seqHistory,
                                                sal_Int32&              nActiveServer   ,
                                        const   ::rtl::OUString&        sServer         );

        /*-****************************************************************************************************//**
            @short      helper methods to read/write  properties from/to ini file
            @descr      Using of Config-Class isn't easy everytime :-(
                        Thats the reason for these helper. State of operation isn't realy important ..
                        but we assert impossible cases or occurred errors!

            @seealso    -

            @param      -
            @return     -

            @onerror    Assertions are shown.
        *//*-*****************************************************************************************************/

        void                    impl_writeUserName              (   const   ::rtl::OUString&        sUserName       );
        void                    impl_writeActiveServer          (           sal_Int32               nActiveServer   );
        void                    impl_writeServerHistory         (   const   SEQUENCE< ::rtl::OUString >& lHistory   );
        void                    impl_writeConnectionType        (   const   ::rtl::OUString&        sConnectionType );
        void                    impl_writeLanguage              (   const   LOCALE&                 aLanguage       );
        void                    impl_writePortHttp              (           sal_Int32               nPort           );
        void                    impl_writePortHttps             (           sal_Int32               nPort           );
        void                    impl_writeSecurityProxy         (   const   ::rtl::OUString&        sSecurityProxy  );
        void                    impl_writeUseProxy              (   const   ::rtl::OUString&        sUseProxy       );
        void                    impl_writeDialog                (   const   ::rtl::OUString&        sDialog         );

        ::rtl::OUString         impl_readUserName               (                                                   );
        sal_Int32               impl_readActiveServer           (                                                   );
        SEQUENCE< ::rtl::OUString > impl_readServerHistory      (                                                   );
        ::rtl::OUString         impl_readConnectionType         (                                                   );
        LOCALE                  impl_readLanguage               (                                                   );
        sal_Int32               impl_readPortHttp               (                                                   );
        sal_Int32               impl_readPortHttps              (                                                   );
        ::rtl::OUString         impl_readSecurityProxy          (                                                   );
        ::rtl::OUString         impl_readUseProxy               (                                                   );
        ::rtl::OUString         impl_readDialog                 (                                                   );

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

        sal_Bool impldbg_checkParameter_LoginDialog (   const   REFERENCE< XMULTISERVICEFACTORY >&  xFactory    );
        sal_Bool impldbg_checkParameter_setTitle    (   const   ::rtl::OUString&                    sTitle      );

    #endif  // #ifdef ENABLE_ASSERTIONS

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        REFERENCE< XMULTISERVICEFACTORY >       m_xFactory          ;   /// reference to factory, which has created this instance
        ::rtl::OUString                         m_sININame          ;   /// full qualified path to profile UNC-notation
        Config*                                 m_pINIManager       ;   /// manager for full access to ini file
        sal_Bool                                m_bInExecuteMode    ;   /// protection against setting of properties during showing of dialog
        cIMPL_Dialog*                           m_pDialog           ;   /// VCL dialog
        tIMPL_DialogData                        m_aPropertySet      ;

};      //  class LoginDialog

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_LOGINDIALOG_LOGINDIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
