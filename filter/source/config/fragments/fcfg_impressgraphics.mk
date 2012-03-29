# *************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
# *************************************************************
all_fragments+=impressgraphics

# -----------------------------------------------
# count = 20
T4_IMPRESSGRAPHICS = \
    bmp_MS_Windows \
    emf_MS_Windows_Metafile \
    eps_Encapsulated_PostScript \
    gif_Graphics_Interchange \
    graphic_HTML \
    graphic_SWF \
    impress_CGM_Computer_Graphics_Metafile \
    jpg_JPEG \
    met_OS2_Metafile \
    pbm_Portable_Bitmap \
    pct_Mac_Pict \
    pgm_Portable_Graymap \
    png_Portable_Network_Graphic \
    ppm_Portable_Pixelmap \
    ras_Sun_Rasterfile \
    svg_Scalable_Vector_Graphics \
    svm_StarView_Metafile \
    tif_Tag_Image_File \
    wmf_MS_Windows_Metafile \
    xpm_XPM

# -----------------------------------------------
# count = 20
F4_IMPRESSGRAPHICS = \
    CGM___Computer_Graphics_Metafile \
    impress_bmp_Export \
    impress_emf_Export \
    impress_eps_Export \
    impress_flash_Export \
    impress_gif_Export \
    impress_html_Export \
    impress_jpg_Export \
    impress_met_Export \
    impress_pbm_Export \
    impress_pct_Export \
    impress_pgm_Export \
    impress_png_Export \
    impress_ppm_Export \
    impress_ras_Export \
    impress_svg_Export \
    impress_svm_Export \
    impress_tif_Export \
    impress_wmf_Export \
    impress_xpm_Export

# -----------------------------------------------
# count = 1
F4_UI_IMPRESSGRAPHICS = \
    impress_html_Export_ui

# -----------------------------------------------
# count = 0
L4_IMPRESSGRAPHICS =

# -----------------------------------------------
# count = 0
C4_IMPRESSGRAPHICS =

# -----------------------------------------------
TYPES_4fcfg_impressgraphics           = $(foreach,i,$(T4_IMPRESSGRAPHICS)    types$/$i.xcu                   )
FILTERS_4fcfg_impressgraphics         = $(foreach,i,$(F4_IMPRESSGRAPHICS)    filters$/$i.xcu                 )
UI_FILTERS_4fcfg_impressgraphics      = $(foreach,i,$(F4_UI_IMPRESSGRAPHICS) $(DIR_LOCFRAG)$/filters$/$i.xcu )
FRAMELOADERS_4fcfg_impressgraphics    = $(foreach,i,$(L4_IMPRESSGRAPHICS)    frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_impressgraphics = $(foreach,i,$(C4_IMPRESSGRAPHICS)    contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_impressgraphics = \
    $(TYPES_4fcfg_impressgraphics) \
    $(FILTERS_4fcfg_impressgraphics) \
    $(UI_FILTERS_4fcfg_impressgraphics) \
    $(FRAMELOADERS_4fcfg_impressgraphics) \
    $(CONTENTHANDLERS_4fcfg_impressgraphics)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_impressgraphics)    
    
ALL_PACKAGES+=impressgraphics
    
