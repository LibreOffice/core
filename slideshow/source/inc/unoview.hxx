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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_UNOVIEW_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_UNOVIEW_HXX

#include "view.hxx"
#include <com/sun/star/uno/Reference.hxx>

#include <vector>

namespace com { namespace sun { namespace star { namespace presentation
{
    class XSlideShowView;
} } } }


/* Definition of UnoView interface */

namespace slideshow
{
    namespace internal
    {
        /** Extend View with UNO interface retrieval.

            This interface extends View with an UNO interface
            retrieval, to be used for Views which are set from
            external API.
         */
        class UnoView : public View
        {
        public:
            /** Retrieve the underlying UNO slide view.
             */
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XSlideShowView > getUnoView() const = 0;

            /** Dispose view

                This needs to be different from Disposable interface,
                as the UNO XComponent also provides a dispose() (only
                with a different calling convention under Windows).
             */
            virtual void _dispose() = 0;

            /** Return whether the sound play back is enabled.
            */
            virtual bool isSoundEnabled() const = 0;

            /** Tell the view whether it may play sounds.  Disabling this
                can be used to prevent different views to play the same
                sounds at the same time.
            */
            virtual void setIsSoundEnabled (const bool bValue) = 0;
        };

        typedef std::shared_ptr< UnoView >      UnoViewSharedPtr;
        typedef std::vector< UnoViewSharedPtr >   UnoViewVector;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_UNOVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
