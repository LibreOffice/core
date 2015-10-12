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

#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX

#include <editeng/unotext.hxx>
#include <editeng/unoedsrc.hxx>


namespace frm
{

    class IEngineTextChangeListener
    {
    public:
        virtual void potentialTextChange( ) = 0;

    protected:
        ~IEngineTextChangeListener() {}
    };

    class ORichTextUnoWrapper : public SvxUnoText
    {
    public:
        ORichTextUnoWrapper( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener );

    protected:
        virtual ~ORichTextUnoWrapper() throw();


    private:
        ORichTextUnoWrapper( const ORichTextUnoWrapper& ) = delete;
        ORichTextUnoWrapper& operator=( const ORichTextUnoWrapper& ) = delete;
    };

    class RichTextEditSource : public SvxEditSource
    {
    private:
        EditEngine&                 m_rEngine;
        SvxTextForwarder*           m_pTextForwarder;
        IEngineTextChangeListener*  m_pTextChangeListener;

    public:
        RichTextEditSource( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener );

        // SvxEditSource
        virtual SvxEditSource*      Clone() const override;
        virtual SvxTextForwarder*   GetTextForwarder() override;
        virtual void                UpdateData() override;

    protected:
        virtual ~RichTextEditSource();

    private:
        RichTextEditSource( const RichTextEditSource& _rSource ) = delete;
        RichTextEditSource& operator=( const RichTextEditSource& ) = delete;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
