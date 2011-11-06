/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
