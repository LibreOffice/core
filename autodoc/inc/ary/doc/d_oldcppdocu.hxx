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

#ifndef ARY_DOC_D_OLDCPPDOCU_HXX
#define ARY_DOC_D_OLDCPPDOCU_HXX
//  KORR_DEPRECATED_3.0

// BASE CLASSES
#include <ary/doc/d_node.hxx>
// USED SERVICES
#include <ary/info/inftypes.hxx>
#include <ary/ary_disp.hxx>

namespace ary
{
namespace info
{
    class AtTag;
    class DocuStore;
}
}




namespace ary
{
namespace doc
{
    using ::ary::info::AtTag;
    using ::ary::info::E_AtTagId;


/** Wrapper class for old C++ documentation format.

    To be replaced by using the standard format.
*/
class OldCppDocu : public Node
{
  public:
    typedef std::vector< DYN AtTag * >	TagList;

    // LIFECYCLE
                        OldCppDocu();
    virtual				~OldCppDocu();

    void		        Store2(
                            info::DocuStore &   o_rDocuStore );

    virtual AtTag *		Create_StdTag(
                            E_AtTagId			i_eId );
    virtual AtTag *		CheckIn_BaseTag();
    virtual AtTag *		CheckIn_ExceptionTag();
    virtual AtTag *		Create_ImplementsTag();
    virtual AtTag *		Create_KeywordTag();
    virtual AtTag *		CheckIn_ParameterTag();
    virtual AtTag *		CheckIn_SeeTag();
    virtual AtTag *		CheckIn_TemplateTag();
    virtual AtTag *		Create_LabelTag();
    virtual AtTag *		Create_DefaultTag();
    virtual AtTag *		Create_SinceTag();          /// @return always the first one created.

    virtual void        Replace_AtShort_By_AtDescr();

    virtual void		Set_Obsolete();
    virtual void		Set_Internal();
    virtual void		Set_Interface()         { bIsInterface = true; }

  // INQUIRY
    const TagList &		Tags() const			{ return aTags; }
    const AtTag &       Short() const;
    bool				IsObsolete() const	    { return bIsObsolete; }
    virtual bool        IsInternal() const;
    virtual bool        IsInterface() const;

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    unsigned char       nTags[ary::info::C_eAtTag_NrOfClasses];

    /** Creates a new AtTag at the end of aTags.
        The index of this new AtTag is inserted in nTags at position
        i_nIndex.
    */
    AtTag * &			NewTag(
                            UINT8				i_nIndex );
    /** Returns the Tag with the position nTags[i_nIndex]
        in aTags.
    */
    AtTag &             GetTag(
                            UINT8				i_nIndex );

    TagList				aTags;
    bool				bIsObsolete;
    bool				bIsInternal;
    bool				bIsInterface;
};




}   // namespace doc
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
