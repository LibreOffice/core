package org.libreoffice.canvas;

import android.graphics.Bitmap;
import android.graphics.Color;

public class ImageUtils {
    /**
     * Convert transparent pixels to gray ones.
     */
    public static Bitmap bitmapToPressed(Bitmap input) {
        Bitmap op = Bitmap.createBitmap(input.getWidth(), input.getHeight(), input.getConfig());
        for(int i=0; i<op.getWidth(); i++){
            for(int j=0; j<op.getHeight(); j++){
                int p = input.getPixel(i, j);
                // assign gray color if the pixel in input is transparent.
                int newColor = Color.alpha(p) == 0 ? Color.argb(255, 200, 200, 200) : p;
                op.setPixel(i, j, newColor);
            }
        }

        return op;
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
