/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_oldcppdocu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:59:37 $
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
    typedef std::vector< DYN AtTag * >  TagList;

    // LIFECYCLE
                        OldCppDocu();
    virtual             ~OldCppDocu();

    void                Store2(
                            info::DocuStore &   o_rDocuStore );

    virtual AtTag *     Create_StdTag(
                            E_AtTagId           i_eId );
    virtual AtTag *     CheckIn_BaseTag();
    virtual AtTag *     CheckIn_ExceptionTag();
    virtual AtTag *     Create_ImplementsTag();
    virtual AtTag *     Create_KeywordTag();
    virtual AtTag *     CheckIn_ParameterTag();
    virtual AtTag *     CheckIn_SeeTag();
    virtual AtTag *     CheckIn_TemplateTag();
    virtual AtTag *     Create_LabelTag();
    virtual AtTag *     Create_DefaultTag();
    virtual AtTag *     Create_SinceTag();          /// @return always the first one created.

    virtual void        Replace_AtShort_By_AtDescr();

    virtual void        Set_Obsolete();
    virtual void        Set_Internal();
    virtual void        Set_Interface()         { bIsInterface = true; }

  // INQUIRY
    const TagList &     Tags() const            { return aTags; }
    const AtTag &       Short() const;
    bool                IsObsolete() const      { return bIsObsolete; }
    virtual bool        IsInternal() const;
    virtual bool        IsInterface() const;

    static const OldCppDocu &
                        Null_();
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
    AtTag * &           NewTag(
                            UINT8               i_nIndex );
    /** Returns the Tag with the position nTags[i_nIndex]
        in aTags.
    */
    AtTag &             GetTag(
                            UINT8               i_nIndex );

    TagList             aTags;
    bool                bIsObsolete;
    bool                bIsInternal;
    bool                bIsInterface;
};




}   // namespace doc
}   // namespace ary
#endif
