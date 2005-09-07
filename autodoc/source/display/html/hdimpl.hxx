/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hdimpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:29:50 $
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

#ifndef ADC_DISPLAY_HDIMPL_HXX
#define ADC_DISPLAY_HDIMPL_HXX



// USED SERVICES
    // BASE CLASSES
#include <udm/html/htmlitem.hxx>
    // COMPONENTS
#include "easywri.hxx"
    // PARAMETERS
#include <cosv/bstream.hxx>
#include <ary/ids.hxx>
#include <ary/ary_disp.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/ce.hxx>
#include <ary/info/codeinfo.hxx>
#include "aryattrs.hxx"     // For compatibility with earlier times, when those funtions were in this header.


namespace ary
{
     namespace cpp
    {
        class Class;
         class DisplayGate;
        class Function;
        class CppDefinition;
    }

    class CodeEntity;
    class QualifiedName;
}
namespace csi
{
     namespace xml
    {
        class Element;
    }
     namespace html
    {
        class Table;
    }
}

namespace adcdisp
{
     class ParameterTable;
}

class OuputPage_Environment;
class Docu_Display;

class HtmlDisplay_Impl
{
  public:
                        ~HtmlDisplay_Impl();

    const OuputPage_Environment &
                        Env() const             { return *pEnv; }

    // ACCESS
    OuputPage_Environment &
                        Env()                   { return *pEnv; }
    EasyWriter &        Easy()                  { return aWriteHelper; }
    csi::xml::Element & CurOut()                { return aWriteHelper.Out(); }

  protected:
                        HtmlDisplay_Impl(
                            OuputPage_Environment &
                                                io_rEnv );
  private:
    // DATA
    OuputPage_Environment *
                        pEnv;
    EasyWriter          aWriteHelper;
};


namespace dshelp
{

void                DisplaySlot(
                        ary::Display &      o_rDisplay,
                        const ary::AryGroup &
                                            i_rGroup,
                        ary::SlotAccessId   i_nSlot );


const char *        PathUp(
                        uintt                   i_nLevels );
const char *        PathPerLevelsUp(
                        uintt                   i_nLevels,
                        const char *            i_nPathBelowDestinationLevel );

const char *        PathPerRoot(
                        const OuputPage_Environment &
                                                i_rEnv,
                        const char *            i_sPathFromRootDir );
const char *        PathPerNamespace(
                        const OuputPage_Environment &
                                                i_rEnv,
                        const char *            i_sPathFromNamespaceDir );

csi::html::Table &  Create_ChildListTable(
                        csi::xml::Element &     o_rParentElement,
                        const char *            i_sTitle,
                        const char *            i_sLabel );
void                Create_ChildListLabel(
                        csi::xml::Element &     o_rParentElement,
                        const char *            i_sLabel );
DYN csi::html::Table &
                    Create_ChildListTable(
                        const char *            i_sTitle );

const char *        HtmlFileName(
                        const char *            i_sPrefix,
                        const char *            i_sEntityName );

inline const char *
ClassFileName( const char * i_sClassLocalName )
    { return HtmlFileName( "c-", i_sClassLocalName); }
inline const char *
EnumFileName( const char * i_sEnumLocalName )
    { return HtmlFileName( "e-", i_sEnumLocalName); }
inline const char *
TypedefFileName( const char * i_sTypedefLocalName )
    { return HtmlFileName( "t-", i_sTypedefLocalName); }
inline const char *
FileFileName( const char * i_sFileLocalName )
    { return HtmlFileName( "f-", i_sFileLocalName); }

const char *        Path2Class(
                        uintt                   i_nLevelsUp,
                        const char *            i_sClassLocalName );

const char *        Path2Child(
                        const char *            i_sFileName,
                        const char *            i_sSubDir = 0 );

const char *        Path2ChildNamespace(
                        const char *            i_sLocalName );

const char *        OperationLink(
                        const udmstri &         i_sOpName,
                        ary::OSid               i_nSignature,
                        const char *            i_sPrePath = "" );
const char *        DataLink(
                        const udmstri &         i_sLocalName,
                        const char *            i_sPrePath = ""  );

inline const char *
OperationLabel( const udmstri &         i_sOpName,
                ary::OSid               i_nSignature )
    { return OperationLink(i_sOpName, i_nSignature) + 1; }     // Skip '#' in front.
inline const char *
DataLabel( const udmstri &         i_sLocalName )
    { return DataLink(i_sLocalName) + 1; }     // Skip '#' in front.


void                Get_LinkedTypeText(
                        csi::xml::Element &     o_rOut,
                        const OuputPage_Environment &
                                                i_rEnv,
                        ary::Tid                i_nId,
                        bool                    i_bWithAbsolutifier = true );


const char *        Link2Ce(
                        const OuputPage_Environment &
                                                i_rEnv,
                        const ary::CodeEntity & i_rCe );

const char *        Link2CppDefinition(
                        const OuputPage_Environment &
                                                i_rEnv,
                        const ary::cpp::CppDefinition &
                                                i_rDef );

const ary::CodeEntity *
                    FindUnambiguousCe(
                        const OuputPage_Environment &
                                                i_rEnv,
                        const ary::QualifiedName &
                                                i_rQuName,
                        const ary::cpp::Class * i_pJustDocumentedClass );

void                ShowDocu_On(
                        csi::xml::Element &     o_rOut,
                        Docu_Display &          io_rDisplay,
                        const ary::RepositoryEntity &
                                                i_rRE );

void                WriteOut_TokenList(
                        csi::xml::Element &     o_rOut,
                        const StringVector &    i_rTokens,
                        const char *            i_sSeparator );

void                EraseLeadingSpace(
                        udmstri &               io_rStr );

/** @param o_bIsConst
    *o_bIsConst will be set to true, if o_bIsConst != 0 and function is const.
    If the function is not const, *o_bIsConst remains unchanged!

    @param o_bIsVirtual
    The same as o_bIsConst.
*/
void                WriteOut_LinkedFunctionText(
                        csi::xml::Element &     o_rTitleOut,
                        adcdisp::ParameterTable &
                                                o_rParameters,
                        const ary::cpp::Function &
                                                i_rFunction,
                        const OuputPage_Environment &
                                                i_rEnv,
                        bool *                  o_bIsConst = 0,
                        bool *                  o_bIsVirtual = 0 );



}   // namespace dshelp

using namespace dshelp;

#endif

