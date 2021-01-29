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

#ifndef INCLUDED_UNOTOOLS_MODULEOPTIONS_HXX
#define INCLUDED_UNOTOOLS_MODULEOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotools/options.hxx>
#include <memory>

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::beans { struct PropertyValue; }

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/
class SvtModuleOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about installation state of modules
    @descr          Use these class to get installation state of different office modules like writer, calc etc
                    Further you can ask for additional information; e.g. name of standard template file, which
                    should be used by corresponding module; or short/long name of these module factory.
    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtModuleOptions final : public utl::detail::Options
{
    public:

        enum class EModule
        {
            WRITER       = 0,
            CALC         = 1,
            DRAW         = 2,
            IMPRESS      = 3,
            MATH         = 4,
            CHART        = 5,
            STARTMODULE  = 6,
            BASIC        = 7,
            DATABASE     = 8,
            WEB          = 9,
            GLOBAL       = 10
        };

        /*ATTENTION:
            If you change these enum ... don't forget to change reading/writing and order of configuration values too!
            See "SvtModuleOptions_Impl::impl_GetSetNames()" and his ctor for further information.
         */
        enum class EFactory
        {
            UNKNOWN_FACTORY = -1,
            WRITER        =  0,
            WRITERWEB     =  1,
            WRITERGLOBAL  =  2,
            CALC          =  3,
            DRAW          =  4,
            IMPRESS       =  5,
            MATH          =  6,
            CHART         =  7,
            STARTMODULE   =  8,
            DATABASE      =  9,
            BASIC         = 10,
            LAST = BASIC
        };

    public:
         SvtModuleOptions();
        virtual ~SvtModuleOptions() override;

        bool        IsModuleInstalled         (       EModule          eModule    ) const;
        OUString GetModuleName             (       EModule          eModule    ) const;
        OUString GetFactoryName            (       EFactory         eFactory   ) const;
        OUString GetFactoryStandardTemplate(       EFactory         eFactory   ) const;
        OUString GetFactoryEmptyDocumentURL(       EFactory         eFactory   ) const;
        OUString GetFactoryDefaultFilter   (       EFactory         eFactory   ) const;
        bool        IsDefaultFilterReadonly   (       EFactory         eFactory   ) const;
        sal_Int32       GetFactoryIcon            (       EFactory         eFactory   ) const;
        static bool ClassifyFactoryByName     ( std::u16string_view sName      ,
                                                          EFactory&        eFactory   );
        void            SetFactoryStandardTemplate(       EFactory         eFactory   ,
                                                    const OUString& sTemplate  );
        void            SetFactoryDefaultFilter   (       EFactory         eFactory   ,
                                                    const OUString& sFilter    );

        /** @short  return the corresponding application ID for the given
                    document service name.
         */
        static EFactory ClassifyFactoryByServiceName(std::u16string_view sName);

        /** @short  return the corresponding application ID for the given
                    short name.
         */
        static EFactory ClassifyFactoryByShortName(const OUString& sName);

        /** @short  return the corresponding application ID for the given properties.

            @descr  Because this search base on filters currently (till we have a better solution)
                    a result is not guaranteed every time. May a filter does not exists for the specified
                    content (but a FrameLoader which is not bound to any application!) ... or
                    the given properties describe a stream (and we make no deep detection inside here!).

            @attention  The module BASIC can't be detected here. Because it does not
                        has an own URL schema.

            @param  sURL
                    the complete URL!

            @param  lMediaDescriptor
                    additional information

            @return A suitable enum value. See EFactory above.
         */
        static EFactory ClassifyFactoryByURL(const OUString&                                                           sURL            ,
                                             const css::uno::Sequence< css::beans::PropertyValue >& lMediaDescriptor);

        /** @short  return the corresponding application ID for the given properties.

            @descr  Here we try to use the list of supported service names of the given model
                    to find out the right application module.

            @attention  The module BASIC can't be detected here. Because it does not
                        support any model/ctrl/view paradigm.

            @param  xModel
                    the document model

            @return A suitable enum value. See EFactory above.
         */
        static EFactory ClassifyFactoryByModel(const css::uno::Reference< css::frame::XModel >& xModel);

        static OUString GetFactoryShortName(EFactory eFactory);

        OUString GetDefaultModuleName() const;

        bool   IsMath     () const;
        bool   IsChart    () const;
        bool   IsCalc     () const;
        bool   IsDraw     () const;
        bool   IsWriter   () const;
        bool   IsImpress  () const;
        static bool   IsBasicIDE () { return true; }
        bool   IsDataBase () const;

        css::uno::Sequence < OUString > GetAllServiceNames();

    private:
        std::shared_ptr<SvtModuleOptions_Impl>   m_pImpl;

};      // class SvtModuleOptions

#endif // INCLUDED_UNOTOOLS_MODULEOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
