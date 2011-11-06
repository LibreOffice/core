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


#ifndef _IMARK_HXX
#define _IMARK_HXX

#include <vcl/keycod.hxx>
#include <calbck.hxx>
#include <pam.hxx>
#include <boost/operators.hpp>
#include <map>

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif


struct SwPosition;

namespace sw { namespace mark
{
    class IMark
        : virtual public SwModify // inherited as interface
        , public ::boost::totally_ordered<IMark>
    {
        public:
            //getters
            virtual const SwPosition& GetMarkPos() const =0;
            // GetOtherMarkPos() is only guaranteed to return a valid
            // reference if IsExpanded() returned true
            virtual const SwPosition& GetOtherMarkPos() const =0;
            virtual const SwPosition& GetMarkStart() const =0;
            virtual const SwPosition& GetMarkEnd() const =0;
            virtual const ::rtl::OUString& GetName() const =0;
            virtual bool IsExpanded() const =0;
            virtual bool IsCoveringPosition(const SwPosition& rPos) const =0;

            //setters
            // not available in IMark
            // inside core, you can cast to MarkBase and use its setters,
            // make sure to update the sortings in Markmanager in this case

            //operators and comparisons (non-virtual)
            bool operator<(const IMark& rOther) const
                { return GetMarkStart() < rOther.GetMarkStart(); }
            bool operator==(const IMark& rOther) const
                { return GetMarkStart() == rOther.GetMarkStart(); }
            bool StartsBefore(const SwPosition& rPos) const
                { return GetMarkStart() < rPos; }
            bool StartsAfter(const SwPosition& rPos) const
                { return GetMarkStart() > rPos; }
            bool EndsBefore(const SwPosition& rPos) const
                { return GetMarkEnd() < rPos; }
            bool EndsAfter(const SwPosition& rPos) const
                { return GetMarkEnd() > rPos; }

            virtual rtl::OUString ToString( ) const =0;
    };

    class IBookmark
        : virtual public IMark
    {
        public:
            virtual const ::rtl::OUString& GetShortName() const =0;
            virtual const KeyCode& GetKeyCode() const =0;
            virtual void SetShortName(const ::rtl::OUString&) =0;
            virtual void SetKeyCode(const KeyCode&) =0;
    };

    class IFieldmark
        : virtual public IMark
    {
        public:
            typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any> parameter_map_t;
            //getters
            virtual ::rtl::OUString GetFieldname() const =0;
            virtual ::rtl::OUString GetFieldHelptext() const =0;
            virtual parameter_map_t* GetParameters() =0;
            virtual const parameter_map_t* GetParameters() const =0;

            //setters
            virtual void SetFieldname(const ::rtl::OUString& rFieldname) =0;
            virtual void SetFieldHelptext(const ::rtl::OUString& rFieldHelptext) =0;
            virtual void Invalidate() = 0;
    };

    class ICheckboxFieldmark
        : virtual public IFieldmark
    {
        public:
            virtual bool IsChecked() const =0;
            virtual void SetChecked(bool checked) =0;
    };
}}
#endif
