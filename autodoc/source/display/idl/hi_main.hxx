/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_main.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:53:29 $
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
#include <ary/idl/ihost_ce.hxx>
    // COMPONENTS
#include "hi_factory.hxx"
    // PARAMETERS


class HtmlEnvironment_Idl;
class HtmlFactory_Idl;
class DocuFile_Html;


class MainDisplay_Idl : public ary::idl::CeHost
{
  public:
                        MainDisplay_Idl(
                            HtmlEnvironment_Idl &
                                                io_rEnv );
    virtual             ~MainDisplay_Idl();

    void                WriteGlobalIndices();

  private:
    // Interface ary::idl::CeHost:
    virtual void        do_Module(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Service(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_SglIfcService(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Interface(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Struct(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Exception(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Enum(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Typedef(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_ConstantsGroup(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_Singleton(
                            const ary::idl::CodeEntity &
                                                i_rData );
    virtual void        do_SglIfcSingleton(
                            const ary::idl::CodeEntity &
                                                i_rData );
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
