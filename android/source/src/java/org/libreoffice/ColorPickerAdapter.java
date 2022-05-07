package org.libreoffice;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Color;
import androidx.recyclerview.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageButton;


public class ColorPickerAdapter extends RecyclerView.Adapter<ColorPickerAdapter.ColorPickerViewHolder> {

    private final Context mContext;
    private final ColorPaletteAdapter colorPaletteAdapter;
    private final ColorPaletteListener colorPaletteListener;
    private final int[] colorList;
    private final int[][] colorPalette = new int[11][8];

    public ColorPickerAdapter(Context mContext, final ColorPaletteAdapter colorPaletteAdapter, ColorPaletteListener colorPaletteListener) {
        this.mContext = mContext;
        this.colorPaletteAdapter = colorPaletteAdapter;
        this.colorPaletteListener = colorPaletteListener;
        Resources r = mContext.getResources();
        this.colorList = r.getIntArray(R.array.fontcolors);
        initializeColorPalette();


    }

    @Override
    public ColorPickerViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View item = LayoutInflater.from(mContext).inflate(R.layout.colorbox, parent, false);
        return new ColorPickerViewHolder(item);

    }

    @Override
    public void onBindViewHolder(final ColorPickerViewHolder holder, int position) {
        holder.colorBox.setBackgroundColor(colorList[position]);

        if (colorPaletteAdapter.getUpperSelectedBox() == position
                && colorPaletteAdapter.getSelectedBox() >= 0) {
            holder.colorBox.setImageResource(R.drawable.ic_done_white_12dp);
        } else {
            holder.colorBox.setImageDrawable(null);
        }

        holder.colorBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                LibreOfficeMainActivity.setDocumentChanged(true);
                setPosition(holder.getAdapterPosition());
                colorPaletteListener.applyColor(colorList[holder.getAdapterPosition()]);
            }
        });
    }

    @Override
    public int getItemCount() {
        return colorList.length;
    }


    private void setPosition(int position) {
        selectSubColor(position, position==0?0:3);
        colorPaletteListener.applyColor(colorList[position]);
        updateAdapter();
    }

    /**
     * Switches to first palette, but doesn't mark any color as selected.
     * Use this if no color in the palette matches the actual one.
     */
    public void unselectColors() {
        colorPaletteAdapter.changePosition(0, -1);
        updateAdapter();
    }

    private void selectSubColor(int position1, int position2) {
        colorPaletteAdapter.setPosition(position1, position2);
    }

    private void initializeColorPalette() {

        for (int i = 0; i < 11; i++) {
            int red = Color.red(colorList[i]);
            int green = Color.green(colorList[i]);
            int blue = Color.blue(colorList[i]);

            int red_tint = red;
            int green_tint = green;
            int blue_tint = blue;

            int red_shade = red;
            int green_shade = green;
            int blue_shade = blue;
            if (i == 0) {
                colorPalette[0][0] = colorList[i];
                for (int k = 1; k < 7; k++) {
                    red_tint = (int) (red_tint + (255 - red_tint) * 0.25);
                    green_tint = (int) (green_tint + (255 - green_tint) * 0.25);
                    blue_tint = (int) (blue_tint + (255 - blue_tint) * 0.25);
                    colorPalette[i][k] = (Color.rgb(red_tint, green_tint, blue_tint));
                }
            } else {
                colorPalette[i][3] = colorList[i];
                for (int k = 2; k >= 0; k--) {
                    red_shade = (int) (red_shade * 0.75);
                    green_shade = (int) (green_shade * 0.75);
                    blue_shade = (int) (blue_shade * 0.75);
                    colorPalette[i][k] = (Color.rgb(red_shade, green_shade, blue_shade));
                }
                for (int k = 4; k < 7; k++) {
                    red_tint = (int) (red_tint + (255 - red_tint) * 0.45);
                    green_tint = (int) (green_tint + (255 - green_tint) * 0.45);
                    blue_tint = (int) (blue_tint + (255 - blue_tint) * 0.45);
                    colorPalette[i][k] = (Color.rgb(red_tint, green_tint, blue_tint));
                }
            }
            colorPalette[i][7] = Color.WHITE; // last one is always white
        }
        colorPaletteAdapter.setColorPalette(colorPalette);
    }

    public void findSelectedTextColor(int color) {
        // try to find and highlight the color in the existing palettes
        for (int i = 0; i < 11; i++) {
            for (int k = 0; k < 8; k++) {
                if (colorPalette[i][k] == color) {
                    colorPaletteAdapter.changePosition(i, k);
                    updateAdapter();
                    return;
                }
            }
        }

        // no color in the palettes matched
        unselectColors();
    }

    private void updateAdapter(){
        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                ColorPickerAdapter.this.notifyDataSetChanged();
            }
        });

    }

    class ColorPickerViewHolder extends RecyclerView.ViewHolder {

        ImageButton colorBox;

        public ColorPickerViewHolder(View itemView) {
            super(itemView);
            this.colorBox = itemView.findViewById(R.id.fontColorBox);
        }
    }
}
