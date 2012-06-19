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

#ifndef ADC_DISPLAY_HFIDMAIN_HXX
#define ADC_DISPLAY_HFIDMAIN_HXX



// USED SERVICES
    // BASE CLASSES
#include <cosv/tpl/processor.hxx>
    // COMPONENTS
#include "hi_factory.hxx"
    // PARAMETERS


class HtmlEnvironment_Idl;
class HtmlFactory_Idl;
class DocuFile_Html;

namespace ary
{
namespace idl
{

    class Module;
    class Service;
    class SglIfcService;
    class Interface;
    class Struct;
    class Exception;
    class Enum;
    class Typedef;
    class ConstantsGroup;
    class Singleton;
    class SglIfcSingleton;

}   // namespace idl
}   // namespace ary


class MainDisplay_Idl : public csv::ProcessorIfc,
                        public csv::ConstProcessor<ary::idl::Module>,
                        public csv::ConstProcessor<ary::idl::Service>,
                        public csv::ConstProcessor<ary::idl::SglIfcService>,
                        public csv::ConstProcessor<ary::idl::Interface>,
                        public csv::ConstProcessor<ary::idl::Struct>,
                        public csv::ConstProcessor<ary::idl::Exception>,
                        public csv::ConstProcessor<ary::idl::Enum>,
                        public csv::ConstProcessor<ary::idl::Typedef>,
                        public csv::ConstProcessor<ary::idl::ConstantsGroup>,
                        public csv::ConstProcessor<ary::idl::Singleton>,
                        public csv::ConstProcessor<ary::idl::SglIfcSingleton>
{
  public:
                        MainDisplay_Idl(
                            HtmlEnvironment_Idl &
                                                io_rEnv );
    virtual             ~MainDisplay_Idl();

    void                WriteGlobalIndices();

    void                Display_NamedEntityHierarchy();

  private:
    // Interface csv::ProcessorIfc:
    virtual void        do_Process(
                            const ary::idl::Module  &     i_client );
    virtual void        do_Process(
                            const ary::idl::Service &     i_client );
    virtual void        do_Process(
                            const ary::idl::SglIfcService &
                                                          i_client );
    virtual void        do_Process(
                            const ary::idl::Interface &   i_client );
    virtual void        do_Process(
                            const ary::idl::Struct  &     i_client );
    virtual void        do_Process(
                            const ary::idl::Exception &   i_client );
    virtual void        do_Process(
                            const ary::idl::Enum &        i_client );
    virtual void        do_Process(
                            const ary::idl::Typedef &     i_client );
    virtual void        do_Process(
                            const ary::idl::ConstantsGroup &
                                                i_client );
    virtual void        do_Process(
                            const ary::idl::Singleton &   i_client );
    virtual void        do_Process(
                            const ary::idl::SglIfcSingleton &
                                                i_client );
    // Locals
    void                do_ServiceDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_SglIfcServiceDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_InterfaceDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_StructDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_ExceptionDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_EnumDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_TypedefDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_SingletonDescr(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Service2s(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Interface2s(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Struct2s(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Exception2s(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Enum2s(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Typedef2s(
                            const ary::idl::CodeEntity &
                                                i_rData );
    void                do_Singleton2s(
                            const ary::idl::CodeEntity &
                                                i_rData );

    const HtmlEnvironment_Idl &
                        Env() const             { return *pEnv; }
    HtmlEnvironment_Idl &
                        Env()                   { return *pEnv; }
    Xml::Element &      CurHtmlOut()            { return pCurFactory->CurOut(); }

    // DATA
    HtmlEnvironment_Idl *
                        pEnv;
    Dyn<DocuFile_Html>  pMyFile;
    HtmlFactory_Idl *   pCurFactory;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
