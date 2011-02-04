# do not add internal filter
#all_fragments+=internalgraphics

# -----------------------------------------------
# count = 27
T4_INTERNALGRAPHICS = \
    bmp_MS_Windows \
    dxf_AutoCAD_Interchange \
    emf_MS_Windows_Metafile \
    eps_Encapsulated_PostScript \
    gif_Graphics_Interchange \
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
# count = 43
F4_INTERNALGRAPHICS = \
    bmp_Export \
    bmp_Import \
    dxf_Import \
    emf_Export \
    emf_Import \
    eps_Export \
    eps_Import \
    gif_Export \
    gif_Import \
    jpg_Export \
    jpg_Import \
    met_Export \
    met_Import \
    pbm_Export \
    pbm_Import \
    pcd_Import_Base \
    pcd_Import_Base4 \
    pcd_Import_Base16 \
    pct_Export \
    pct_Import \
    pcx_Import \
    pgm_Export \
    pgm_Import \
    png_Export \
    png_Import \
    ppm_Export \
    ppm_Import \
    psd_Import \
    ras_Export \
    ras_Import \
    sgf_Import \
    sgv_Import \
    svg_Export \
    svg_Import \
    svm_Export \
    svm_Import \
    tga_Import \
    tif_Export \
    tif_Import \
    wmf_Export \
    wmf_Import \
    xbm_Import \
    xpm_Export \
    xpm_Import

# -----------------------------------------------
# count = 0
L4_INTERNALGRAPHICS =

# -----------------------------------------------
# count = 0
C4_INTERNALGRAPHICS =

# -----------------------------------------------
TYPES_4fcfg_internalgraphics           = $(foreach,i,$(T4_INTERNALGRAPHICS) types$/$i.xcu                   )
FILTERS_4fcfg_internalgraphics         = $(foreach,i,$(F4_INTERNALGRAPHICS) internalgraphicfilters$/$i.xcu  )
FRAMELOADERS_4fcfg_internalgraphics    = $(foreach,i,$(L4_INTERNALGRAPHICS) frameloaders$/$i.xcu            )
CONTENTHANDLERS_4fcfg_internalgraphics = $(foreach,i,$(C4_INTERNALGRAPHICS) contenthandlers$/$i.xcu         )

# -----------------------------------------------
# needed to get dependencies inside global makefile work!
ALL_4fcfg_internalgraphics = \
    $(TYPES_4fcfg_internalgraphics) \
    $(FILTERS_4fcfg_internalgraphics) \
    $(FRAMELOADERS_4fcfg_internalgraphics) \
    $(CONTENTHANDLERS_4fcfg_internalgraphics)
    
# do not add internal filter
# ALL_UI_FILTERS+=
    
# do not add internal filter
# ALL_PACKAGES+=internalgraphics
    
