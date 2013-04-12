function Slideshow() {

    var mSlides = 0;
    var mCurrentSlide;

    var mNotes = [];
    var mPreviews = [];

    this.setLength = function( aLength ) {
        mSlides = aLength;
    }

    this.setCurrentSlide = function( aSlide ) {
        mCurrentSlide = aSlide;
    }

    this.getCurrentSlide = function() {
        return mCurrentSlide;
    }

    this.putPreview = function( aSlide, aImage ) {
        mPreviews[ aSlide ] = aImage;
    }

    this.putNotes = function( aSlide, aNotes ) {
        mNotes[ aSlide ] = aNotes;
    }

    this.getPreview = function( aSlide ) {
        return mPreviews[ aSlide ];
    }

}