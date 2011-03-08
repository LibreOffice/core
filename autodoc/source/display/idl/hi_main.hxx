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
