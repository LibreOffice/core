/*************************************************************************
 *
 *  $RCSfile: hfi_typetext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-15 13:35:06 $
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

#ifndef ADC_DISPLAY_HFI_TYPETEXT_HXX
#define ADC_DISPLAY_HFI_TYPETEXT_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS


class HF_IdlTypeText : public HtmlFactory_Idl
{
  public:
    enum E_Index { use_for_javacompatible_index };

                        HF_IdlTypeText(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut,
                            bool                i_bWithLink,
                            const client *      i_pScopeGivingCe = 0 );
                        HF_IdlTypeText(
                            Environment &       io_rEnv,
                            E_Index             e );
    virtual             ~HF_IdlTypeText();

    void                Produce_byData(
                            ary::idl::Type_id   i_idType ) const;
    void                Produce_byData(
                            ary::idl::Ce_id     i_idCe ) const;
    void                Produce_byData(
                            const String &      i_sFullName ) const;
    void                Produce_LinkInDocu(
                            const String &      i_scope,
                            const String &      i_name,
                            const String &      i_member ) const;
    void                Produce_LocalLinkInDocu(
                            const String &      i_member ) const;

    /// Produce the first link for Java-help understood index entries.
    void                Produce_IndexLink(
                            Xml::Element &      o_out,
                            const client &      i_ce ) const;
    /** Produce the second link for Java-help understood index entries.
        For members this will be a link to their owner (this function is
        used), else see @->Produce_IndexSecondEntryLink();
    */
    void                Produce_IndexOwnerLink(
                            Xml::Element &      o_out,
                            const client &      i_owner ) const;
    /** Produce the second link for Java-help understood index entries.
        For non- members this will again be a link to to the entry itself
        (this function is used), else see @->Produce_IndexOwnerLink();
    */
    void                Produce_IndexSecondEntryLink(
                            Xml::Element &      o_out,
                            const client &      i_ce ) const;
  private:
    // Locals
    enum E_Existence
    {
        exists_dontknow,
        exists_yes,
        exists_no
    };

    void                produce_FromStd(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member,
                            int                 i_sequenceCount,
                            E_Existence         i_ceExists,
                            ary::idl::Type_id   i_nTemplateType = ary::idl::Type_id::Null_() ) const;
    void                produce_BuiltIn(
                            const String &      i_type,
                            int                 i_sequenceCount ) const;
    void                produce_IndexLink(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member,
                            bool                i_bIsOwner ) const;
    int                 count_Sequences(
                            const char *        i_sFullType ) const;
    void                start_Sequence(
                            int                 i_count ) const;
    void                finish_Sequence(
                            int                 i_count ) const;
    void                errorOut_UnresolvedLink(
                            const char *        i_name ) const;
    void                errorOut_UnresolvedLink(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member ) const;
    void                errorOut_UnresolvedLink(
                            const String &      i_module,
                            const String &      i_ce,
                            const String &      i_member ) const;
    const ary::idl::Module *
                        referingModule() const;
    const client *      referingCe() const;

    // DATA
    mutable const client *
                        pReferingCe;
    bool                bWithLink;
};



// IMPLEMENTATION



#endif


