all_fragments+=drawgraphics

# -----------------------------------------------
# count = 29
T4_DRAWGRAPHICS = \
    bmp_MS_Windows \
    dxf_AutoCAD_Interchange \
    emf_MS_Windows_Metafile \
    eps_Encapsulated_PostScript \
    gif_Graphics_Interchange \
    graphic_HTML \
    graphic_SWF \
    jpg_JPEG \
    met_OS2_Metafile \
    pbm_Portable_Bitmap \
    pcd_Photo_CD_Base \
    pcd_Photo_CD_Base16 \
    pcd_Photo_CD_Base4 \
    pct_Mac_Pict \
    pcx_Zsoft_Paintbrush \
    pgm_Portable_Graymap \
    png_Portable_Network_Graphic \
    ppm_Portable_Pixelmap \
    psd_Adobe_Photoshop \
    ras_Sun_Rasterfile \
    sgf_StarOffice_Writer_SGF \
    sgv_StarDraw_20 \
    svg_Scalable_Vector_Graphics \
    svm_StarView_Metafile \
    tga_Truevision_TARGA \
    tif_Tag_Image_File \
    wmf_MS_Windows_Metafile \
    xbm_X_Consortium \
    xpm_XPM

# -----------------------------------------------
# count = 45
F4_DRAWGRAPHICS = \
    BMP___MS_Windows \
    DXF___AutoCAD_Interchange \
    EMF___MS_Windows_Metafile \
    EPS___Encapsulated_PostScript \
    GIF___Graphics_Interchange \
    JPG___JPEG \
    MET___OS_2_Metafile \
    PBM___Portable_Bitmap \
    PCT___Mac_Pict \
    PCX___Zsoft_Paintbrush \
    PGM___Portable_Graymap \
    PNG___Portable_Network_Graphic \
    PPM___Portable_Pixelmap \
    PSD___Adobe_Photoshop \
    RAS___Sun_Rasterfile \
    SGF___StarOffice_Writer_SGF \
    SGV___StarDraw_2_0 \
    SVG___Scalable_Vector_Graphics \
    SVM___StarView_Metafile \
    SVG___Scalable_Vector_Graphics \
    TGA___Truevision_TARGA \
    TIF___Tag_Image_File \
    WMF___MS_Windows_Metafile \
    XBM___X_Consortium \
    XPM \
    draw_PCD_Photo_CD_Base \
    draw_PCD_Photo_CD_Base16 \
    draw_PCD_Photo_CD_Base4 \
    draw_bmp_Export \
    draw_emf_Export \
    draw_eps_Export \
    draw_flash_Export \
    draw_gif_Export \
    draw_html_Export \
    draw_jpg_Export \
    draw_met_Export \
    draw_pbm_Export \
    draw_pct_Export \
    draw_pgm_Export \
    draw_png_Export \
    draw_ppm_Export \
    draw_ras_Export \
    draw_svg_Export \
    draw_svm_Export \
    draw_tif_Export \
    draw_wmf_Export \
    draw_xpm_Export

# -----------------------------------------------
# count = 1
F4_UI_DRAWGRAPHICS = \
    draw_html_Export_ui

# -----------------------------------------------
# count = 0
L4_DRAWGRAPHICS =

# -----------------------------------------------
# count = 0
C4_DRAWGRAPHICS =

# -----------------------------------------------
TYPES_4fcfg_drawgraphics           = $(foreach,i,$(T4_DRAWGRAPHICS)    types$/$i.xcu                    )
FILTERS_4fcfg_drawgraphics         = $(foreach,i,$(F4_DRAWGRAPHICS)    filters$/$i.xcu                  )
UI_FILTERS_4fcfg_drawgraphics      = $(foreach,i,$(F4_UI_DRAWGRAPHICS) $(DIR_LOCFRAG)$/filters$/$i.xcu  )
FRAMELOADERS_4fcfg_drawgraphics    = $(foreach,i,$(L4_DRAWGRAPHICS)    frameloaders$/$i.xcu             )
CONTENTHANDLERS_4fcfg_drawgraphics = $(foreach,i,$(C4_DRAWGRAPHICS)    contenthandlers$/$i.xcu          )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_drawgraphics = \
    $(TYPES_4fcfg_drawgraphics) \
    $(FILTERS_4fcfg_drawgraphics) \
    $(UI_FILTERS_4fcfg_drawgraphics) \
    $(FRAMELOADERS_4fcfg_drawgraphics) \
    $(CONTENTHANDLERS_4fcfg_drawgraphics)
    
ALL_UI_FILTERS+=$(UI_FILTERS_4fcfg_drawgraphics)    
    
ALL_PACKAGES+=drawgraphics
    
