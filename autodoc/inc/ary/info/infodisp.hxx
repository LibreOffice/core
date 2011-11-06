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



#ifndef ARY_INFO_INFODISP_HXX
#define ARY_INFO_INFODISP_HXX
//  KORR_DEPRECATED_3.0

// BASE CLASSES
// USED SERVICES




namespace ary
{
namespace info
{
    class StdTag;
    class BaseTag;
    class ExceptionTag;
    class ImplementsTag;
    class KeywordTag;
    class ParameterTag;
    class SeeTag;
    class TemplateTag;
    class LabelTag;
    class SinceTag;
    class DT_Text;
    class DT_MaybeLink;
    class DT_Whitespace;
    class DT_Eol;
    class DT_Xml;



/** Displaying an ary::doc::OldCppDocu.

    @descr
    This class is an interface, but the functions are defaulted,
    to do nothing. so a derived class needn't implement all of them.
*/
class DocuDisplay
{
  public:
    virtual             ~DocuDisplay() {}

    virtual void        Display_StdTag(
                            const StdTag &      i_rData ) = 0;
    virtual void        Display_BaseTag(
                            const BaseTag &     i_rData ) = 0;
    virtual void        Display_ExceptionTag(
                            const ExceptionTag &
                                                i_rData ) = 0;
    virtual void        Display_ImplementsTag(
                            const ImplementsTag &
                                                i_rData ) = 0;
    virtual void        Display_KeywordTag(
                            const KeywordTag &  i_rData ) = 0;
    virtual void        Display_ParameterTag(
                            const ParameterTag &
                                                i_rData ) = 0;
    virtual void        Display_SeeTag(
                            const SeeTag &      i_rData ) = 0;
    virtual void        Display_TemplateTag(
                            const TemplateTag & i_rData ) = 0;
    virtual void        Display_LabelTag(
                            const LabelTag &    i_rData ) = 0;
    virtual void        Display_SinceTag(
                            const ary::info::SinceTag &
                                                i_rData ) = 0;
    virtual void        Display_DT_Text(
                            const DT_Text &     i_rData ) = 0;
    virtual void        Display_DT_MaybeLink(
                            const DT_MaybeLink& i_rData ) = 0;
    virtual void        Display_DT_Whitespace(
                            const DT_Whitespace &
                                                i_rData ) = 0;
    virtual void        Display_DT_Eol(
                            const DT_Eol &      i_rData ) = 0;
    virtual void        Display_DT_Xml(
                            const ary::info::DT_Xml &
                                                i_rData ) = 0;
};




}
}
#endif
