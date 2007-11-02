/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_main.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:40:17 $
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
