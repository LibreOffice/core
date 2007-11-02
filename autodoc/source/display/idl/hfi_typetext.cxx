/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_typetext.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:38:19 $
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

#include <precomp.h>
#include "hfi_typetext.hxx"


// NOT FULLY DEFINED SERVICES
#include <string.h>
#include <ary/idl/i_type.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/ik_ce.hxx>
#include <adc_cl.hxx>
#include <adc_msg.hxx>
#include "hi_linkhelper.hxx"


inline const ary::idl::Module *
HF_IdlTypeText::referingModule() const
{
    if (pReferingCe == 0)
        return Env().Linker().Search_CurModule();
    else
        return &Env().Data().Find_Module(pReferingCe->NameRoom());
}

inline const HF_IdlTypeText::client *
HF_IdlTypeText::referingCe() const
{
    return pReferingCe;
}


HF_IdlTypeText::HF_IdlTypeText( Environment &       io_rEnv,
                                Xml::Element &      o_rOut,
                                bool                i_bWithLink,
                                const client *      i_pScopeGivingCe )
    :   HtmlFactory_Idl(io_rEnv, &o_rOut),
        pReferingCe( i_pScopeGivingCe ),
        bWithLink(i_bWithLink)
{
}

HF_IdlTypeText::HF_IdlTypeText( Environment &       io_rEnv,
                                E_Index             )
    :   HtmlFactory_Idl(io_rEnv, 0),
        pReferingCe( 0 ),
        bWithLink(true)
{
}

HF_IdlTypeText::~HF_IdlTypeText()
{
}

void
HF_IdlTypeText::Produce_byData( ary::idl::Type_id i_idType ) const
{
    StringVector        aModule_;
    String              sName;
    ce_id               nCe;
    int                 nSequenceCount = 0;
    csv::erase_container(aModule_);

    const ary::idl::Type &
        rType = Env().Data().Find_Type(i_idType);
    Env().Data().Get_TypeText(aModule_, sName, nCe, nSequenceCount, rType);
    ary::idl::Type_id
        nTemplateType = rType.TemplateParameterType();

    if ( Env().Data().IsBuiltInOrRelated(rType) )
    {
        produce_BuiltIn(sName,nSequenceCount);
    }
    else
    {
        produce_FromStd( aModule_,
                         sName,
                         String::Null_(),
                         nSequenceCount,
                         (nCe.IsValid() ? exists_yes : exists_no),
                         nTemplateType );
    }
}

void
HF_IdlTypeText::Produce_byData( ary::idl::Ce_id i_idCe ) const
{
    StringVector        aModule_;
    String              sCe;
    String              sMember;
    csv::erase_container(aModule_);

    const ary::idl::CodeEntity &
        rCe = Env().Data().Find_Ce(i_idCe);
    Env().Data().Get_CeText(aModule_, sCe, sMember, rCe);
    produce_FromStd(aModule_, sCe, sMember, 0, exists_yes);
}

void
HF_IdlTypeText::Produce_byData( const String & i_sFullName ) const
{
    if ( strncmp(i_sFullName,"http://", 7) == 0 )
    {
        CurOut()
            >> *new Html::Link(i_sFullName)
                << i_sFullName;
        return;
    }

    StringVector        aModule_;
    String              sCe,
                        sMember;
    int                 nSequence = 0;
    String              sTypeText;
    csv::erase_container(aModule_);

    const ary::idl::Module *
        pScopeModule = referingModule();
    if (pScopeModule == 0)
    {
        // SYNTAX_ERR, but rather logical error: Missing module.
        CurOut() << i_sFullName;
        // KORR_FUTURE
        // How to put a message about this?
        // errorOut_UnresolvedLink(i_sFullName);
        return;
    }

    const char * sTypeStart = strrchr(i_sFullName,'<');
    if ( sTypeStart != 0 )
    {
        const char * sTypeEnd = strchr(i_sFullName,'>');
        if (sTypeEnd == 0)
        {   // SYNTAX_ERR
            CurOut() << i_sFullName;
            // KORR_FUTURE
            // How to put a message about this?
            // errorOut_UnresolvedLink(i_sFullName);
            return;
        }

        nSequence = count_Sequences(i_sFullName);
        sTypeStart++;
        sTypeText.assign(sTypeStart, sTypeEnd-sTypeStart);
    }
    else
    {
        sTypeText = i_sFullName;
    }

    csv::erase_container(aModule_);
    bool bFound =   // KORR : Check the semantics of this, see if ce really exists, if it is a member?
        Env().Data().Search_Ce( aModule_,
                                sCe,sMember,
                                sTypeText,
                                *pScopeModule );
    if (NOT bFound)
    {
        if ( strchr(sTypeText,':') == 0
             AND
             *sTypeText.c_str() != 'X' )    // This is a HACK, make this correct!
        {
            Produce_LocalLinkInDocu(sTypeText);
            return;
        }
        CurOut() << i_sFullName;
        // KORR
        // How to put a message about this?
        // errorOut_UnresolvedLink(i_sFullName);
        return;
    }

    produce_FromStd(aModule_, sCe, sMember, nSequence, exists_yes);
}

void
HF_IdlTypeText::Produce_LinkInDocu( const String &      i_scope,
                                    const String &      i_name,
                                    const String &      i_member ) const
{
    StringVector        aModule_;
    String              sName;
    csv::erase_container(aModule_);

    const ary::idl::Module *
        pScopeModule = referingModule();
    if (pScopeModule == 0)
    {
        // SYNTAX_ERR, but rather logical error: Missing module.
        CurOut() << i_scope << "::" << i_name;
        if (NOT i_member.empty())
            CurOut() << "::" << i_member;
        return;
    }

    bool
        bFound = Env().Data().Search_CesModule( aModule_,
                                                i_scope,
                                                i_name,
                                                *pScopeModule );
    if (NOT bFound)
    {
        CurOut() << i_scope << "::" << i_name;
        if (NOT i_member.empty())
            CurOut() << "::" << i_member;
        return;
    }
    produce_FromStd(aModule_, i_name, i_member, 0, exists_yes);
}

void
HF_IdlTypeText::Produce_LocalLinkInDocu( const String & i_member ) const
{
    StringVector        aModule_;
    String              sName;
    csv::erase_container(aModule_);

    csv_assert(referingCe() != 0);
    if ( referingModule() == Env().Linker().Search_CurModule() )
    {
        StreamLock slLink(200);
        if (referingCe()->SightLevel() == ary::idl::sl_Member)
        {
            slLink() << "#" << i_member;
        }
        else
        {
            slLink() << referingCe()->LocalName()
                     << ".html#"
                     << i_member;
        }
        CurOut()
            >> *new Html::Link(slLink().c_str())
               << i_member;
        return;
    }

    String sDummyMember;
    Env().Data().Get_CeText(aModule_, sName, sDummyMember, *referingCe());
    produce_FromStd(aModule_, sName, i_member, 0, exists_yes);
}

void
HF_IdlTypeText::Produce_IndexLink( Xml::Element &   o_out,
                                   const client &   i_ce ) const
{
    StringVector        aModule_;
    String              sCe;
    String              sMember;
    csv::erase_container(aModule_);

    Out().Enter(o_out);

    Env().Data().Get_CeText(aModule_, sCe, sMember, i_ce);
    produce_IndexLink(aModule_, sCe, sMember, false);

    Out().Leave();
}

void
HF_IdlTypeText::Produce_IndexOwnerLink( Xml::Element &  o_out,
                                        const client &  i_owner ) const
{
    StringVector        aModule_;
    String              sCe;
    String              sMember;
    csv::erase_container(aModule_);

    Out().Enter(o_out);

    if (i_owner.Owner().IsValid())
    {
        Env().Data().Get_CeText(aModule_, sCe, sMember, i_owner);
        produce_IndexLink(aModule_, sCe, sMember, true);
    }
    else
    {   // global namespace:

        CurOut()
            << "."
            >> *new Html::Link("../module-ix.html")
               << "global namespace";
    }


    Out().Leave();
}

void
HF_IdlTypeText::Produce_IndexSecondEntryLink( Xml::Element &      o_out,
                                              const client &      i_ce ) const
{
    StringVector        aModule_;
    String              sCe;
    String              sMember;
    csv::erase_container(aModule_);

    Out().Enter(o_out);

    Env().Data().Get_CeText(aModule_, sCe, sMember, i_ce);
    produce_IndexLink(aModule_, sCe, sMember, true);
    Out().Leave();
}


void
HF_IdlTypeText::produce_FromStd( const StringVector & i_module,
                                 const String &       i_ce,
                                 const String &       i_member,
                                 int                  i_sequenceCount,
                                 E_Existence          i_ceExists,
                                 ary::idl::Type_id    i_nTemplateType ) const
{
    if (i_ceExists == exists_no)
    {
        if ( is_ExternLink(i_module) )
        {
            produce_ExternLink(i_module,i_ce,i_member,i_sequenceCount,i_nTemplateType);
            return;
        }
        errorOut_UnresolvedLink(i_module, i_ce, i_member);
    }

    output::Node &
        rCeNode = Env().OutputTree().Provide_Node(i_module);
    output::Position
        aTargetPos(rCeNode);
    bool
        bShowModule = rCeNode != Env().CurPosition().RelatedNode()
                            ?   i_module.size() > 0
                            :   false;
    bool
        bUseMember = NOT i_member.empty();
    bool
        bLink2Module = i_ceExists == exists_yes;
    bool
        bLink2Ce = i_ceExists == exists_yes;
    bool
        bLink2Member = NOT Env().Is_MemberExistenceCheckRequired()
                       AND i_ceExists == exists_yes;
    bool
        bHasCeOrName = NOT i_ce.empty();

    if (i_sequenceCount > 0)
        start_Sequence(i_sequenceCount);

    StreamLock  aLink(300);
    StreamStr & rLink = aLink();

    // Produce output: module
    if (bShowModule)
    {
        int nMax = i_module.size() - 1;
        int nCount = 0;
        StringVector::const_iterator
            itm = i_module.begin();
        for ( ;
              nCount < nMax;
              ++itm, ++nCount )
        {
            CurOut() << "::" << *itm;
        }

        CurOut() << "::";
        if (bLink2Module)
        {
            aTargetPos.Set_File(output::ModuleFileName());
            Env().Linker().Get_Link2Position(rLink, aTargetPos);
            CurOut()
                >> *new Html::Link( rLink.c_str() )
                    << *itm;
            rLink.reset();
        }
        else
        {
            CurOut() << *itm;
        }

        if (bHasCeOrName)
            CurOut() << "::";
    }   // end if (bShowModule)

    // CodeEntity and member:
    aTargetPos.Set_File( rLink << i_ce << ".html" << c_str );
    rLink.reset();

    if (bHasCeOrName)
    {
        if (bLink2Ce)
        {
            Env().Linker().Get_Link2Position(rLink, aTargetPos);
            CurOut()
                >> *new Html::Link(rLink.c_str())
                    << i_ce;
            rLink.reset();
        }
        else
        {
            CurOut() << i_ce;
        }

        if (i_nTemplateType.IsValid())
        {
            CurOut() << "< ";

            HF_IdlTypeText
                aTemplateParamWriter(Env(), CurOut(), true, pReferingCe);
            aTemplateParamWriter.Produce_byData(i_nTemplateType);

            CurOut() << " >";
        }

        if (bUseMember)
        {
            CurOut() << "::";

            if (bLink2Member)
            {
                bool bFunction = strstr(i_member,"()") != 0;
                String sMember( i_member );
                if (bFunction)
                    sMember.assign(i_member.c_str(), sMember.length()-2);

                Env().Linker().Get_Link2Member(rLink, aTargetPos, sMember);
                CurOut()
                    >> *new Html::Link(rLink.c_str())
                        << i_member;
                rLink.reset();
            }
            else
            {
                CurOut()
                    << i_member;
            }
        }   // endif (bUseMember)
    }   // endif (bHasCeOrName)

    if (i_sequenceCount > 0)
        finish_Sequence(i_sequenceCount);
}

void
HF_IdlTypeText::produce_BuiltIn( const String &      i_type,
                                 int                 i_sequenceCount ) const
{
    if (i_sequenceCount > 0)
        start_Sequence(i_sequenceCount);
    CurOut() << i_type;
    if (i_sequenceCount > 0)
        finish_Sequence(i_sequenceCount);
}

void
HF_IdlTypeText::produce_IndexLink( const StringVector & i_module,
                                   const String &       i_ce,
                                   const String &       i_member,
                                   bool                 i_bIsOwner ) const
{
    output::Node &
        rCeNode = Env().OutputTree().Provide_Node(i_module);
    output::Position
        aTargetPos(rCeNode);
    bool
        bShowModule = i_bIsOwner OR i_module.size() > 0 AND i_ce.empty();
    bool
        bShowNonModule = NOT bShowModule OR i_bIsOwner AND NOT i_ce.empty();
    bool
        bUseMember = NOT i_member.empty();

    StreamLock  aLink(300);
    StreamStr & rLink = aLink();

    // Produce output: module
    if (bShowModule)
    {
        if (i_bIsOwner)
        {
            int nMax = bShowNonModule ? i_module.size() : i_module.size() - 1;
            int nCount = 0;
            for ( StringVector::const_iterator itm = i_module.begin();
                  nCount < nMax;
                  ++itm, ++nCount )
            {
                CurOut() << "::" << *itm;
            }
            CurOut() << ":: .";
        }

        if (NOT bShowNonModule)
        {
            aTargetPos.Set_File(output::ModuleFileName());
            Env().Linker().Get_Link2Position(rLink, aTargetPos);
            CurOut()
                >> *new Html::Link( rLink.c_str() )
                    >> *new Html::Bold
                        << i_module.back();
            rLink.reset();
        }
    }   // end if (bShowModule)

    if (bShowNonModule)
    {
        aTargetPos.Set_File( rLink << i_ce << ".html" << c_str );
        rLink.reset();

        if (bUseMember)
        {
            bool bFunction = strstr(i_member,"()") != 0;
            String sMember( i_member );
            if (bFunction)
                sMember.assign(i_member.c_str(), sMember.length()-2);
            Env().Linker().Get_Link2Member(rLink, aTargetPos, sMember);
            CurOut()
                >> *new Html::Link(rLink.c_str())
                    >> *new Html::Bold
                        << i_member;
            rLink.reset();
        }
        else
        {
            Env().Linker().Get_Link2Position(rLink, aTargetPos);
            if (i_bIsOwner)
            {
                CurOut()
                    >> *new Html::Link(rLink.c_str())
                       << i_ce;
            }
            else
            {
                CurOut()
                    >> *new Html::Link(rLink.c_str())
                           >> *new Html::Bold
                            << i_ce;
            }
            rLink.reset();
        }
    }   // endif (bHasCeOrName)
}

int
HF_IdlTypeText::count_Sequences( const char * i_sFullType ) const
{
    int ret = 0;

    for ( const char * pCount = i_sFullType;
          *pCount != 0;
           )
    {
        pCount = strstr(pCount,"sequence");
        if (pCount != 0)
        {
            pCount += sizeof("sequence");   // = strlen(sequence) + 1 for '<'.
            if ( *(pCount-1) == '\0' )
            {
                // SYNTAX_ERR
                return 0;
            }
            ++ret;
        }
    }   // end for

    return ret;
}

void
HF_IdlTypeText::start_Sequence( int i_count ) const
{
    csv_assert( i_count > 0 );
    for (int i = 0; i < i_count; ++i )
    {
        CurOut() << "sequence< ";
    }
}

void
HF_IdlTypeText::finish_Sequence( int i_count ) const
{
    csv_assert( i_count > 0 );
    for (int i = 0; i < i_count; ++i )
    {
        CurOut() << " >";
    }
}

void
HF_IdlTypeText::errorOut_UnresolvedLink( const char *        i_name ) const
{
    StreamLock slFile(1000);

    // KORR
    // Handle links in cited documentation from other entities.
    slFile() << Env().CurPageCe_AsText();
    slFile().pop_back(5);
    slFile() << ".idl";

    // KORR
    // Retrieve, correct line number.
    TheMessages().Out_UnresolvedLink( i_name,
                                      slFile().c_str(),
                                      0 );
}

void
HF_IdlTypeText::errorOut_UnresolvedLink( const StringVector & i_module,
                                         const String &       i_ce,
                                         const String &       i_member ) const
{
    StreamLock slName(500);

    if (i_module.size() > 0)
    {
        slName().operator_join(i_module.begin(), i_module.end(), "::");
         if (NOT i_ce.empty())
            slName() << "::";
    }
    if (NOT i_ce.empty())
    {
        slName() << i_ce;
        if (NOT i_member.empty())
            slName() << "::" << i_member;
    }
    errorOut_UnresolvedLink(slName().c_str());
}

void
HF_IdlTypeText::errorOut_UnresolvedLink( const String &      i_module,
                                         const String &      i_ce,
                                         const String &      i_member ) const
{
    StreamLock slName(500);

    if (i_module.size() > 0)
    {
        slName() << i_module;
         if (NOT i_ce.empty())
            slName() << "::";
    }
    if (NOT i_ce.empty())
    {
        slName() << i_ce;
        if (NOT i_member.empty())
            slName() << "::" << i_member;
    }
    errorOut_UnresolvedLink(slName().c_str());
}

bool
HF_IdlTypeText::is_ExternLink( const StringVector & i_module ) const
{
    const autodoc::CommandLine &
        rCmdLine = autodoc::CommandLine::Get_();
    uintt nExtNspLength = rCmdLine.ExternNamespace().length();
    if (nExtNspLength == 0)
        return false;

    StreamStr s(1000);
    s << "::";
    s.operator_join( i_module.begin(),
                     i_module.end(),
                     "::" );

    if (s.length() < nExtNspLength)
        return false;
    return ( strncmp( rCmdLine.ExternNamespace().c_str(),
                      s.c_str(),
                      nExtNspLength ) == 0 );
}

void
HF_IdlTypeText::produce_ExternLink( const StringVector & i_module,
                                    const String &       i_ce,
                                    const String &       i_member,
                                    int                  i_sequenceCount,
                                    ary::idl::Type_id    i_nTemplateType ) const
{
    // KORR
    // Look again at this code and take some time.

    StreamLock  aLink(1000);
    StreamStr & rLink = aLink();

    rLink << autodoc::CommandLine::Get_().ExternRoot();
    rLink.operator_join( i_module.begin(),
                         i_module.end(),
                         "/" );
    rLink << '/'
          << i_ce
          << ".html";
    if (i_member.length() > 0)
        rLink << "/#" << i_member;

    if (i_sequenceCount > 0)
        start_Sequence(i_sequenceCount);

    // module
    int nMax = i_module.size();
    int nCount = 0;
    StringVector::const_iterator
        itm = i_module.begin();
    for ( ;
          nCount < nMax;
          ++itm, ++nCount )
    {
        CurOut() << "::" << *itm;
    }
    CurOut() << "::";


    // CodeEntity
    if (i_member.length() == 0)
    {
        CurOut()
           >> *new Html::Link(rLink.c_str())
              << i_ce;
    }
    else
    {
        CurOut()
            << i_ce;
    }

    if (i_nTemplateType.IsValid())
    {
        CurOut() << "< ";

        HF_IdlTypeText
            aTemplateParamWriter(Env(), CurOut(), true, pReferingCe);
        aTemplateParamWriter.Produce_byData(i_nTemplateType);

        CurOut() << " >";
    }

    // Member
    if (i_member.length() > 0)
    {
        CurOut()
            >> *new Html::Link(rLink.c_str())
                << i_member;
    }

    if (i_sequenceCount > 0)
        finish_Sequence(i_sequenceCount);
}
