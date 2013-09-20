/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#define INCLUDED_SVTOOLS_MODULEOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <salhelper/singletonref.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <unotools/options.hxx>

#define FEATUREFLAG_BASICIDE                0x00000020
#define FEATUREFLAG_MATH                    0x00000100
#define FEATUREFLAG_CHART                   0x00000200
#define FEATUREFLAG_CALC                    0x00000800
#define FEATUREFLAG_DRAW                    0x00001000
#define FEATUREFLAG_WRITER                  0x00002000
#define FEATUREFLAG_IMPRESS                 0x00008000
#define FEATUREFLAG_INSIGHT                 0x00010000

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/
class SvtModuleOptions_Impl;

/*-************************************************************************************************************//**
    @short          collect information about installation state of modules
    @descr          Use these class to get installation state of different office modules like writer, calc etc
                    Further you can ask for additional information; e.g. name of standard template file, which
                    should be used by corresponding module; or short/long name of these module factory.

    @implements     -
    @base           -

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtModuleOptions : public utl::detail::Options
{
    public:

        enum EModule
        {
            E_SWRITER       = 0,
            E_SCALC         = 1,
            E_SDRAW         = 2,
            E_SIMPRESS      = 3,
            E_SMATH         = 4,
            E_SCHART        = 5,
            E_SSTARTMODULE  = 6,
            E_SBASIC        = 7,
            E_SDATABASE     = 8,
            E_SWEB          = 9,
            E_SGLOBAL       = 10
        };

        /*ATTENTION:
            If you change these enum ... don't forget to change reading/writing and order of configuration values too!
            See "SvtModuleOptions_Impl::impl_GetSetNames()" and his ctor for further information.
         */
        enum EFactory
        {
            E_UNKNOWN_FACTORY = -1,
            E_WRITER        =  0,
            E_WRITERWEB     =  1,
            E_WRITERGLOBAL  =  2,
            E_CALC          =  3,
            E_DRAW          =  4,
            E_IMPRESS       =  5,
            E_MATH          =  6,
            E_CHART         =  7,
            E_STARTMODULE   =  8,
            E_DATABASE      =  9,
            E_BASIC         = 10

        };

    public:

         SvtModuleOptions();
        virtual ~SvtModuleOptions();

        sal_Bool        IsModuleInstalled         (       EModule          eModule    ) const;
        OUString GetModuleName             (       EModule          eModule    ) const;
        OUString GetFactoryName            (       EFactory         eFactory   ) const;
        OUString GetFactoryStandardTemplate(       EFactory         eFactory   ) const;
        OUString GetFactoryEmptyDocumentURL(       EFactory         eFactory   ) const;
        OUString GetFactoryDefaultFilter   (       EFactory         eFactory   ) const;
        sal_Bool        IsDefaultFilterReadonly   (       EFactory         eFactory   ) const;
        sal_Int32       GetFactoryIcon            (       EFactory         eFactory   ) const;
        static sal_Bool ClassifyFactoryByName     ( const OUString& sName      ,
                                                          EFactory&        eFactory   );
        void            SetFactoryStandardTemplate(       EFactory         eFactory   ,
                                                    const OUString& sTemplate  );
        void            SetFactoryDefaultFilter   (       EFactory         eFactory   ,
                                                    const OUString& sFilter    );

        //_______________________________________

        /** @short  return the corresponding application ID for the given
                    document service name.
         */
        static EFactory ClassifyFactoryByServiceName(const OUString& sName);

        //_______________________________________

        /** @short  return the corresponding application ID for the given
                    short name.
         */
        static EFactory ClassifyFactoryByShortName(const OUString& sName);

        //_______________________________________

        /** @short  return the corresponding application ID for the given properties.

            @descr  Because this search base on filters currently (till we have a better solution)
                    a result is not guaranteed everytimes. May a filter does not exists for the specified
                    content (but a FrameLoader which is not bound to any application!) ... or
                    the given properties describe a stream (and we make no deep detection inside here!).

            @attention  The module BASIC cant be detected here. Because it does not
                        has an own URL schema.

            @param  sURL
                    the complete URL!

            @param  lMediaDescriptor
                    additional information

            @return A suitable enum value. See EFactory above.
         */
        static EFactory ClassifyFactoryByURL(const OUString&                                                           sURL            ,
                                             const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lMediaDescriptor);

        //_______________________________________

        /** @short  return the corresponding application ID for the given properties.

            @descr  Here we try to use the list of supported service names of the given model
                    to find out the right application module.

            @attention  The module BASIC cant be detected here. Because it does not
                        support any model/ctrl/view paradigm.

            @param  xModel
                    the document model

            @return A suitable enum value. See EFactory above.
         */
        static EFactory ClassifyFactoryByModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel);

        static OUString GetFactoryShortName(EFactory eFactory);

        OUString GetDefaultModuleName();

        sal_Bool   IsMath     () const;
        sal_Bool   IsChart    () const;
        sal_Bool   IsCalc     () const;
        sal_Bool   IsDraw     () const;
        sal_Bool   IsWriter   () const;
        sal_Bool   IsImpress  () const;
        sal_Bool   IsBasicIDE () const;
        sal_Bool   IsDataBase () const;

        ::com::sun::star::uno::Sequence < OUString > GetAllServiceNames();

    private:
        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& impl_GetOwnStaticMutex();

        /*Attention

            Don't initialize these static member in these header!
            a) Double defined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtModuleOptions_Impl*   m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtModuleOptions

#endif  // #ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
