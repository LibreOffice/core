/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_OSL_DETAIL_COMPONENT_DEFINES_H
#define INCLUDED_OSL_DETAIL_COMPONENT_DEFINES_H

/* Experimental direct constructor calls, under construction */

/* FIXME: Rather than hardcoded, this should be generated from
   solenv/bin/native-code.py */

#define LO_URE_CURRENT_ENV 1 /*TODO*/

// sax/source/expatwrap/expwrap.component
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_xml_dot_sax_dot_FastParser 1 /*TODO*/
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_xml_dot_sax_dot_FastParser com_sun_star_comp_extensions_xml_sax_FastParser_get_implementation
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_xml_dot_sax_dot_Parser 1 /*TODO*/
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_xml_dot_sax_dot_Parser com_sun_star_comp_extensions_xml_sax_ParserExpat_get_implementation
// sfx2/util/sfx.component
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_document_dot_DocumentProperties 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_document_dot_DocumentProperties SfxDocumentMetaData_get_implementation
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_frame_dot_OfficeFrameLoader 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_frame_dot_OfficeFrameLoader com_sun_star_comp_office_FrameLoader_get_implementation
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_frame_dot_SynchronousFrameLoader 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_frame_dot_SynchronousFrameLoader  com_sun_star_comp_office_FrameLoader_get_implementation
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_frame_dot_DocumentTemplates 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_frame_dot_DocumentTemplates com_sun_star_comp_sfx2_DocumentTemplates_get_implementation
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_frame_dot_GlobalEventBroadcaster 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_frame_dot_GlobalEventBroadcaster com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_frame_dot_theGlobalEventBroadcaster 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_frame_dot_theGlobalEventBroadcaster com_sun_star_comp_sfx2_GlobalEventBroadcaster_get_implementation
// svtools/util/svt_dot_component
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_graphic_dot_GraphicProvider 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_graphic_dot_GraphicProvider com_sun_star_comp_graphic_GraphicProvider_get_implementation
// svx/util/svx_dot_component
#define LO_URE_CTOR_ENV_com_dot_sun_dot_star_dot_drawing_dot_CustomShapeEngine 1
#define LO_URE_CTOR_FUN_com_dot_sun_dot_star_dot_drawing_dot_CustomShapeEngine com_sun_star_drawing_EnhancedCustomShapeEngine_get_implementation

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
