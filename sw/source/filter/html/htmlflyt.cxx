/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "htmlfly.hxx"


#define TE(t,p,c) (sal_uInt8)( HTML_OUT_##t | HTML_POS_##p | HTML_CNTNR_##c )

sal_uInt8 aHTMLOutFrmPageFlyTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        
        TE(TBLNODE, BEFORE, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE)       
    },
    {
        
        TE(TBLNODE, BEFORE, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  PREFIX, NONE),      
        TE(MULTICOL,PREFIX, NONE),      
        TE(MULTICOL,PREFIX, DIV)        
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE),      
        TE(DIV,     PREFIX, NONE)       
    },
    {
        
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, PREFIX, NONE),      
        TE(GRFNODE, PREFIX, NONE),      
        TE(GRFNODE, PREFIX, SPAN)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, PREFIX, NONE),      
        TE(OLENODE, PREFIX, NONE),      
        TE(OLENODE, PREFIX, SPAN)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, PREFIX, NONE),      
        TE(OLENODE, PREFIX, NONE),      
        TE(OLENODE, PREFIX, SPAN)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLENODE, PREFIX, NONE),      
        TE(OLENODE, PREFIX, NONE),      
        TE(OLEGRF,  PREFIX, SPAN)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  PREFIX, NONE),      
        TE(OLEGRF,  PREFIX, NONE),      
        TE(OLEGRF,  PREFIX, SPAN)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(AMARQUEE,PREFIX, NONE),      
        TE(AMARQUEE,PREFIX, NONE),      
        TE(GRFFRM,  PREFIX, SPAN)       
    },
    {
        
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, PREFIX, NONE),      
        TE(CONTROL, PREFIX, NONE),      
        
        TE(CONTROL, INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  PREFIX, NONE),      
        TE(GRFFRM,  PREFIX, NONE),      
        TE(GRFFRM,  PREFIX, SPAN)       
    }
};

sal_uInt8 aHTMLOutFrmParaFrameTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        
        TE(TBLNODE, BEFORE, NONE),      
        TE(TBLNODE, BEFORE, NONE),      
        TE(TBLNODE, BEFORE, NONE),      
        TE(TBLNODE, BEFORE, NONE)       
    },
    {
        
        TE(TBLNODE, BEFORE, NONE),      
        TE(DIV,     BEFORE, NONE),      
        TE(DIV,     BEFORE, NONE),      
        TE(TBLNODE, BEFORE, NONE)       
    },
    {
        
        TE(GRFFRM,  BEFORE, NONE),      
        TE(GRFFRM,  BEFORE, NONE),      
        TE(MULTICOL,BEFORE, NONE),      
        TE(MULTICOL,BEFORE, DIV)        
    },
    {
        
        TE(GRFFRM,  BEFORE, NONE),      
        TE(DIV,     BEFORE, NONE),      
        TE(SPACER,  BEFORE, NONE),      
        TE(SPACER,  BEFORE, NONE)       
    },
    {
        
        TE(GRFFRM,  BEFORE, NONE),      
        TE(DIV,     BEFORE, NONE),      
        TE(DIV,     BEFORE, NONE),      
        TE(DIV,     BEFORE, NONE)       
    },
    {
        
        TE(GRFNODE, BEFORE, NONE),      
        TE(GRFNODE, BEFORE, NONE),      
        TE(GRFNODE, BEFORE, NONE),      
        TE(GRFNODE, BEFORE, NONE)       
    },
    {
        
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE)       
    },
    {
        
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE)       
    },
    {
        
        TE(OLEGRF,  BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE),      
        TE(OLENODE, BEFORE, NONE),      
        TE(OLEGRF,  BEFORE, NONE)       
    },
    {
        
        TE(OLEGRF,  BEFORE, NONE),      
        TE(OLEGRF,  BEFORE, NONE),      
        TE(OLEGRF,  BEFORE, NONE),      
        TE(OLEGRF,  BEFORE, NONE)       
    },
    {
        
        
        TE(GRFFRM,  BEFORE, NONE),      
        TE(AMARQUEE,BEFORE, NONE),      
        TE(AMARQUEE,BEFORE, NONE),      
        TE(GRFFRM,  BEFORE, NONE)       
    },
    {
        
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, BEFORE, NONE),      
        TE(CONTROL, BEFORE, NONE),      
        
        TE(CONTROL, BEFORE, NONE)       
    },
    {
        
        TE(GRFFRM,  BEFORE, NONE),      
        TE(GRFFRM,  BEFORE, NONE),      
        TE(GRFFRM,  BEFORE, NONE),      
        TE(GRFFRM,  BEFORE, NONE)       
    }
};

sal_uInt8 aHTMLOutFrmParaPrtAreaTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        
        TE(TBLNODE, INSIDE, NONE),      
        TE(TBLNODE, INSIDE, NONE),      
        TE(TBLNODE, INSIDE, NONE),      
        TE(TBLNODE, INSIDE, NONE)       
    },
    {
        
        TE(TBLNODE, INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(MULTICOL,INSIDE, NONE),      
        TE(MULTICOL,INSIDE, SPAN)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPACER,  INSIDE, NONE),      
        TE(SPACER,  INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE)       
    },
    {
        
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(AMARQUEE,INSIDE, NONE),      
        TE(AMARQUEE,INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE)       
    },
    {
        
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE),      
        
        TE(CONTROL, INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE)       
    }
};

sal_uInt8 aHTMLOutFrmParaOtherTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        
        TE(TBLNODE, BEFORE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE)       
    },
    {
        
        TE(TBLNODE, BEFORE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(MULTICOL,INSIDE, NONE),      
        TE(MULTICOL,INSIDE, SPAN)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE),      
        TE(SPAN,    INSIDE, NONE)       
    },
    {
        
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, SPAN)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, SPAN)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, SPAN)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, SPAN)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, SPAN)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(AMARQUEE,INSIDE, NONE),      
        TE(AMARQUEE,INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, SPAN)       
    },
    {
        
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE),      
        
        TE(CONTROL, INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, SPAN)       
    }
};

sal_uInt8 aHTMLOutFrmAsCharTable[MAX_FRMTYPES][MAX_BROWSERS] =
{
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(MULTICOL,INSIDE, NONE),      
        TE(MULTICOL,INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(SPACER,  INSIDE, NONE),      
        TE(SPACER,  INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE)       
    },
    {
        
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE),      
        TE(GRFNODE, INSIDE, NONE)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE)       
    },
    {
        
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLENODE, INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE)       
    },
    {
        
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE),      
        TE(OLEGRF,  INSIDE, NONE)       
    },
    {
        
        
        TE(MARQUEE, INSIDE, NONE),      
        TE(MARQUEE, INSIDE, NONE),      
        TE(MARQUEE, INSIDE, NONE),      
        TE(MARQUEE, INSIDE, NONE)       
    },
    {
        
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE),      
        TE(CONTROL, INSIDE, NONE)       
    },
    {
        
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE),      
        TE(GRFFRM,  INSIDE, NONE)       
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
