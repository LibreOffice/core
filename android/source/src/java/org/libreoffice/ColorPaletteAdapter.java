package org.libreoffice;

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.ImageButton;


public class ColorPaletteAdapter extends RecyclerView.Adapter<ColorPaletteAdapter.ColorPaletteViewHolder> {

    int[][] color_palette;
    Context mContext;
    int upperSelectedBox = -1;
    int selectedBox = 0;
    boolean animate;
    ColorPaletteListener colorPaletteListener;

    public ColorPaletteAdapter(Context mContext, ColorPaletteListener colorPaletteListener) {
        this.mContext = mContext;
        this.color_palette = new int[11][8];
        this.colorPaletteListener = colorPaletteListener;
    }

    @Override
    public ColorPaletteViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        View item = LayoutInflater.from(mContext).inflate(R.layout.colorbox, parent, false);
        ColorPaletteViewHolder holder = new ColorPaletteViewHolder(item);
        return holder;
    }


    public int getSelectedBox() {
        return selectedBox;
    }

    @Override
    public void onBindViewHolder(final ColorPaletteViewHolder holder, int position) {

        holder.colorBox.setBackgroundColor(color_palette[upperSelectedBox][position]);
        if (selectedBox == position) {
            holder.colorBox.setImageResource(R.drawable.ic_done_all_white_12dp);
        } else {
            holder.colorBox.setImageDrawable(null);
        }

        holder.colorBox.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                setPosition(holder.getAdapterPosition());
            }
        });
        if (animate) //it will only animate when the upper color box is selected
            setAnimation(holder.colorBox);

    }

    private void setAnimation(View viewToAnimate) {
        Animation animation = AnimationUtils.loadAnimation(mContext, android.R.anim.fade_in);
        viewToAnimate.startAnimation(animation);
    }

    @Override
    public int getItemCount() {
        return color_palette[0].length;
    }

    private void setPosition(int position) {
        this.selectedBox = position;
        colorPaletteListener.applyColor(color_palette[upperSelectedBox][position]);
        animate = false;
        updateAdapter();
    }

    public void setPosition(int upperSelectedBox, int position) {
        if (this.upperSelectedBox != upperSelectedBox) {
            this.upperSelectedBox = upperSelectedBox;
            this.selectedBox = position;
            colorPaletteListener.applyColor(color_palette[upperSelectedBox][position]);
            animate = true;
            updateAdapter();
        }
    }

    /*
        this is for InvalidationHandler when .uno:FontColor is captured
     */
    public void changePosition(int upperSelectedBox, int position) {
            if(this.upperSelectedBox != upperSelectedBox){
                this.upperSelectedBox = upperSelectedBox;
                animate=true;
            }

            this.selectedBox = position;

            updateAdapter();

    }

    public void setColorPalette(int[][] color_palette, int position1, int position2) {
        this.color_palette = color_palette;
        setPosition(position1, position2);
    }

    private void updateAdapter(){

        LOKitShell.getMainHandler().post(new Runnable() {
            @Override
            public void run() {
                ColorPaletteAdapter.this.notifyDataSetChanged();
            }
        });
    }


    class ColorPaletteViewHolder extends RecyclerView.ViewHolder {

        ImageButton colorBox;

        public ColorPaletteViewHolder(View itemView) {
            super(itemView);
            colorBox = (ImageButton) itemView.findViewById(R.id.fontColorBox);
        }
    }


}