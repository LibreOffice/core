/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_IMARK_HXX
#define INCLUDED_SW_INC_IMARK_HXX

#include <vcl/keycod.hxx>
#include "calbck.hxx"
#include "pam.hxx"
#include <map>
#include <memory>
#include "swdllapi.h"

namespace sw { namespace mark
{
    enum class InsertMode
    {
        New,
        CopyText,
    };

    class SW_DLLPUBLIC IMark
        : virtual public sw::BroadcastingModify // inherited as interface
    {
        protected:
            IMark() = default;

        public:
            //getters
            virtual const SwPosition& GetMarkPos() const =0;
            // GetOtherMarkPos() is only guaranteed to return a valid
            // reference if IsExpanded() returned true
            virtual const SwPosition& GetOtherMarkPos() const =0;
            virtual const SwPosition& GetMarkStart() const =0;
            virtual const SwPosition& GetMarkEnd() const =0;
            virtual const OUString& GetName() const =0;
            virtual bool IsExpanded() const =0;
            virtual bool IsCoveringPosition(const SwPosition& rPos) const =0;

            //setters
            // not available in IMark
            // inside core, you can cast to MarkBase and use its setters,
            // make sure to update the sorting in Markmanager in this case

            virtual OUString ToString( ) const =0;
            virtual void dumpAsXml(xmlTextWriterPtr pWriter) const = 0;
        private:
            IMark(IMark const &) = delete;
            IMark &operator =(IMark const&) = delete;
    };

    class SW_DLLPUBLIC IBookmark
        : virtual public IMark
    {
        protected:
            IBookmark() = default;

        public:
            virtual const OUString& GetShortName() const =0;
            virtual const vcl::KeyCode& GetKeyCode() const =0;
            virtual void SetShortName(const OUString&) =0;
            virtual void SetKeyCode(const vcl::KeyCode&) =0;
            virtual bool IsHidden() const =0;
            virtual const OUString& GetHideCondition() const =0;
            virtual void Hide(bool hide) =0;
            virtual void SetHideCondition(const OUString&) =0;
        private:
            IBookmark(IBookmark const &) = delete;
            IBookmark &operator =(IBookmark const&) = delete;
    };

    class SW_DLLPUBLIC IFieldmark
        : virtual public IMark
    {
        protected:
            IFieldmark() = default;

        public:
            typedef std::map< OUString, css::uno::Any> parameter_map_t;
            //getters
            virtual OUString GetFieldname() const =0;
            virtual OUString GetFieldHelptext() const =0;
            virtual parameter_map_t* GetParameters() =0;
            virtual const parameter_map_t* GetParameters() const =0;

            //setters
            virtual void SetFieldname(const OUString& rFieldname) =0;
            virtual void SetFieldHelptext(const OUString& rFieldHelptext) =0;
            virtual void Invalidate() = 0;
        private:
            IFieldmark(IFieldmark const &) = delete;
            IFieldmark &operator =(IFieldmark const&) = delete;
    };

    class SW_DLLPUBLIC ICheckboxFieldmark
        : virtual public IFieldmark
    {
        protected:
            ICheckboxFieldmark() = default;

        public:
            virtual bool IsChecked() const =0;
            virtual void SetChecked(bool checked) =0;
        private:
            ICheckboxFieldmark(ICheckboxFieldmark const &) = delete;
            ICheckboxFieldmark &operator =(ICheckboxFieldmark const&) = delete;
    };

    class SW_DLLPUBLIC IDateFieldmark
        : virtual public IFieldmark
    {
        protected:
            IDateFieldmark() = default;

        public:
            virtual OUString GetContent() const = 0;
            virtual void ReplaceContent(const OUString& sNewContent) = 0;

            virtual std::pair<bool, double> GetCurrentDate() const = 0;
            virtual void SetCurrentDate(double fDate) = 0;
            virtual OUString GetDateInStandardDateFormat(double fDate) const = 0;

    private:
            IDateFieldmark(ICheckboxFieldmark const &) = delete;
            IDateFieldmark &operator =(ICheckboxFieldmark const&) = delete;
    };

    OUString ExpandFieldmark(IFieldmark* pBM);

}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
