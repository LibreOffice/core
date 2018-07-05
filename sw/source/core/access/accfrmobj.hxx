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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRMOBJ_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCFRMOBJ_HXX

#include <vcl/vclptr.hxx>

class SwAccessibleMap;
class SwFrame;
class SdrObject;
namespace vcl { class Window; }
class SwRect;

namespace sw { namespace access {

class SwAccessibleChild
{
    public:
        SwAccessibleChild();
        ~SwAccessibleChild();
        explicit SwAccessibleChild( const SdrObject* pDrawObj );
        explicit SwAccessibleChild( const SwFrame* pFrame );
        explicit SwAccessibleChild( vcl::Window* pWindow );
        SwAccessibleChild( const SwFrame* pFrame,
                           const SdrObject* pDrawObj,
                           vcl::Window* pWindow );

        SwAccessibleChild(SwAccessibleChild const &) = default;
        SwAccessibleChild(SwAccessibleChild &&) = default;
        SwAccessibleChild & operator =(SwAccessibleChild const &) = default;
        SwAccessibleChild & operator =(SwAccessibleChild &&) = default;

        SwAccessibleChild& operator=( const SdrObject* pDrawObj );
        SwAccessibleChild& operator=( const SwFrame* pFrame );
        SwAccessibleChild& operator=( vcl::Window* pWindow );

        bool operator==( const SwAccessibleChild& r ) const;

        bool IsValid() const;

        const SwFrame* GetSwFrame() const { return mpFrame; }
        const SdrObject* GetDrawObject() const { return mpDrawObj; }
        vcl::Window* GetWindow() const { return mpWindow; }

        const SwFrame* GetParent( const bool bInPagePreview ) const;

        bool IsAccessible( bool bPagePreview ) const;
        bool IsBoundAsChar() const;

        bool IsVisibleChildrenOnly() const;
        SwRect GetBox( const SwAccessibleMap& rAccMap ) const;
        SwRect GetBounds( const SwAccessibleMap& rAccMap ) const;

        /** indicating, if accessible child is included even, if the corresponding
            object is not visible. */
        bool AlwaysIncludeAsChild() const;

    private:
        const SwFrame* mpFrame;
        const SdrObject* mpDrawObj;
        VclPtr<vcl::Window> mpWindow;

        void Init( const SdrObject* pDrawObj );
        void Init( const SwFrame* pFrame );
        void Init( vcl::Window* pWindow );
};

} } // eof of namespace sw::access

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
