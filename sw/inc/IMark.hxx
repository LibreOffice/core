/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bookmrk.hxx,v $
 * $Revision: 1.11 $
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
#ifndef _IMARK_HXX
#define _IMARK_HXX

#include <vcl/keycod.hxx>
#include <calbck.hxx>
#include <pam.hxx>
#include <boost/operators.hpp>

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
            //getters
            virtual ::rtl::OUString GetFieldname() const =0;
            virtual ::rtl::OUString GetFieldHelptext() const =0;

            //setters
            virtual void SetFieldname(const ::rtl::OUString& rFieldname) =0;
            virtual void SetFieldHelptext(const ::rtl::OUString& rFieldHelptext) =0;
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
