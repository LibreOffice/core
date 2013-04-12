function updatePreview( aSlideshow ) {
    var aOutputImage = document.getElementById('preview');
    var aPreview = aSlideshow.getPreview( aSlideshow.getCurrentSlide() );

    aOutputImage.src = "data:image/png;base64," + aPreview;
    console.log( "Set image : " + aPreview );

    document.getElementById('current_slide').innerHTML = aSlideshow.getCurrentSlide();
}