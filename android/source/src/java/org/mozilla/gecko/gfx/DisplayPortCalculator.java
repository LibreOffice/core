/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.mozilla.gecko.gfx;

import android.graphics.PointF;
import android.graphics.RectF;
import android.util.Log;

import org.libreoffice.LOKitShell;
import org.libreoffice.LibreOfficeMainActivity;
import org.mozilla.gecko.util.FloatUtils;

final class DisplayPortCalculator {
    private static final String LOGTAG = DisplayPortCalculator.class.getSimpleName();
    private static final PointF ZERO_VELOCITY = new PointF(0, 0);

    // Keep this in sync with the TILEDLAYERBUFFER_TILE_SIZE defined in gfx/layers/TiledLayerBuffer.h
    private static final int TILE_SIZE = 256;

    private final DisplayPortStrategy sStrategy;

    DisplayPortCalculator(LibreOfficeMainActivity context) {
        sStrategy = new VelocityBiasStrategy(context);
    }

    DisplayPortMetrics calculate(ImmutableViewportMetrics metrics, PointF velocity) {
        return sStrategy.calculate(metrics, (velocity == null ? ZERO_VELOCITY : velocity));
    }

    boolean aboutToCheckerboard(ImmutableViewportMetrics metrics, PointF velocity, DisplayPortMetrics displayPort) {
        if (displayPort == null) {
            return true;
        }
        return sStrategy.aboutToCheckerboard(metrics, (velocity == null ? ZERO_VELOCITY : velocity), displayPort);
    }

    private static abstract class DisplayPortStrategy {
        /** Calculates a displayport given a viewport and panning velocity. */
        public abstract DisplayPortMetrics calculate(ImmutableViewportMetrics metrics, PointF velocity);
        /** Returns true if a checkerboard is about to be visible and we should not throttle drawing. */
        public abstract boolean aboutToCheckerboard(ImmutableViewportMetrics metrics, PointF velocity, DisplayPortMetrics displayPort);
    }

    /**
     * Expand the given margins such that when they are applied on the viewport, the resulting rect
     * does not have any partial tiles, except when it is clipped by the page bounds. This assumes
     * the tiles are TILE_SIZE by TILE_SIZE and start at the origin, such that there will always be
     * a tile at (0,0)-(TILE_SIZE,TILE_SIZE)).
     */
    private static DisplayPortMetrics getTileAlignedDisplayPortMetrics(RectF margins, ImmutableViewportMetrics metrics) {
        float left = metrics.viewportRectLeft - margins.left;
        float top = metrics.viewportRectTop - margins.top;
        float right = metrics.viewportRectRight + margins.right;
        float bottom = metrics.viewportRectBottom + margins.bottom;
        left = (float) Math.max(metrics.pageRectLeft, TILE_SIZE * Math.floor(left / TILE_SIZE));
        top = (float) Math.max(metrics.pageRectTop, TILE_SIZE * Math.floor(top / TILE_SIZE));
        right = (float) Math.min(metrics.pageRectRight, TILE_SIZE * Math.ceil(right / TILE_SIZE));
        bottom = (float) Math.min(metrics.pageRectBottom, TILE_SIZE * Math.ceil(bottom / TILE_SIZE));
        return new DisplayPortMetrics(left, top, right, bottom);
    }

    /**
     * Adjust the given margins so if they are applied on the viewport in the metrics, the resulting rect
     * does not exceed the page bounds. This code will maintain the total margin amount for a given axis;
     * it assumes that margins.left + metrics.getWidth() + margins.right is less than or equal to
     * metrics.getPageWidth(); and the same for the y axis.
     */
    private static RectF shiftMarginsForPageBounds(RectF margins, ImmutableViewportMetrics metrics) {
        // check how much we're overflowing in each direction. note that at most one of leftOverflow
        // and rightOverflow can be greater than zero, and at most one of topOverflow and bottomOverflow
        // can be greater than zero, because of the assumption described in the method javadoc.
        float leftOverflow = metrics.pageRectLeft - (metrics.viewportRectLeft - margins.left);
        float rightOverflow = (metrics.viewportRectRight + margins.right) - metrics.pageRectRight;
        float topOverflow = metrics.pageRectTop - (metrics.viewportRectTop - margins.top);
        float bottomOverflow = (metrics.viewportRectBottom + margins.bottom) - metrics.pageRectBottom;

        // if the margins overflow the page bounds, shift them to other side on the same axis
        if (leftOverflow > 0) {
            margins.left -= leftOverflow;
            margins.right += leftOverflow;
        } else if (rightOverflow > 0) {
            margins.right -= rightOverflow;
            margins.left += rightOverflow;
        }
        if (topOverflow > 0) {
            margins.top -= topOverflow;
            margins.bottom += topOverflow;
        } else if (bottomOverflow > 0) {
            margins.bottom -= bottomOverflow;
            margins.top += bottomOverflow;
        }
        return margins;
    }

    /**
     * This class implements the variation with a small fixed-size margin with velocity bias.
     * In this variation, the default margins are pretty small relative to the view size, but
     * they are affected by the panning velocity. Specifically, if we are panning on one axis,
     * we remove the margins on the other axis because we are likely axis-locked. Also once
     * we are panning in one direction above a certain threshold velocity, we shift the buffer
     * so that it is almost entirely in the direction of the pan, with a little bit in the
     * reverse direction.
     */
    private static class VelocityBiasStrategy extends DisplayPortStrategy {
        // The length of each axis of the display port will be the corresponding view length
        // multiplied by this factor.
        private final float SIZE_MULTIPLIER;
        // The velocity above which we apply the velocity bias
        private final float VELOCITY_THRESHOLD;
        // How much of the buffer to keep in the reverse direction of the velocity
        private final float REVERSE_BUFFER;
        // If the visible rect is within the danger zone we start redrawing to minimize
        // checkerboarding. the danger zone amount is a linear function of the form:
        //    viewportsize * (base + velocity * incr)
        // where base and incr are configurable values.
        private final float DANGER_ZONE_BASE_X_MULTIPLIER;
        private final float DANGER_ZONE_BASE_Y_MULTIPLIER;
        private final float DANGER_ZONE_INCR_X_MULTIPLIER;
        private final float DANGER_ZONE_INCR_Y_MULTIPLIER;

        VelocityBiasStrategy(LibreOfficeMainActivity context) {
            SIZE_MULTIPLIER = 2.0f;
            VELOCITY_THRESHOLD = LOKitShell.getDpi(context) * 0.032f;
            REVERSE_BUFFER = 0.2f;
            DANGER_ZONE_BASE_X_MULTIPLIER = 1.0f;
            DANGER_ZONE_BASE_Y_MULTIPLIER = 1.0f;
            DANGER_ZONE_INCR_X_MULTIPLIER = 0;
            DANGER_ZONE_INCR_Y_MULTIPLIER = 0;
        }

        /**
         * Split the given amounts into margins based on the VELOCITY_THRESHOLD and REVERSE_BUFFER values.
         * If the velocity is above the VELOCITY_THRESHOLD on an axis, split the amount into REVERSE_BUFFER
         * and 1.0 - REVERSE_BUFFER fractions. The REVERSE_BUFFER fraction is set as the margin in the
         * direction opposite to the velocity, and the remaining fraction is set as the margin in the direction
         * of the velocity. If the velocity is lower than VELOCITY_THRESHOLD, split the amount evenly into the
         * two margins on that axis.
         */
        private RectF velocityBiasedMargins(float xAmount, float yAmount, PointF velocity) {
            RectF margins = new RectF();

            if (velocity.x > VELOCITY_THRESHOLD) {
                margins.left = xAmount * REVERSE_BUFFER;
            } else if (velocity.x < -VELOCITY_THRESHOLD) {
                margins.left = xAmount * (1.0f - REVERSE_BUFFER);
            } else {
                margins.left = xAmount / 2.0f;
            }
            margins.right = xAmount - margins.left;

            if (velocity.y > VELOCITY_THRESHOLD) {
                margins.top = yAmount * REVERSE_BUFFER;
            } else if (velocity.y < -VELOCITY_THRESHOLD) {
                margins.top = yAmount * (1.0f - REVERSE_BUFFER);
            } else {
                margins.top = yAmount / 2.0f;
            }
            margins.bottom = yAmount - margins.top;

            return margins;
        }

        public DisplayPortMetrics calculate(ImmutableViewportMetrics metrics, PointF velocity) {
            float displayPortWidth = metrics.getWidth() * SIZE_MULTIPLIER;
            float displayPortHeight = metrics.getHeight() * SIZE_MULTIPLIER;

            // but if we're panning on one axis, set the margins for the other axis to zero since we are likely
            // axis locked and won't be displaying that extra area.
            if (Math.abs(velocity.x) > VELOCITY_THRESHOLD && FloatUtils.fuzzyEquals(velocity.y, 0)) {
                displayPortHeight = metrics.getHeight();
            } else if (Math.abs(velocity.y) > VELOCITY_THRESHOLD && FloatUtils.fuzzyEquals(velocity.x, 0)) {
                displayPortWidth = metrics.getWidth();
            }

            // we need to avoid having a display port that is larger than the page, or we will end up
            // painting things outside the page bounds (bug 729169).
            displayPortWidth = Math.min(displayPortWidth, metrics.getPageWidth());
            displayPortHeight = Math.min(displayPortHeight, metrics.getPageHeight());
            float horizontalBuffer = displayPortWidth - metrics.getWidth();
            float verticalBuffer = displayPortHeight - metrics.getHeight();

            // split the buffer amounts into margins based on velocity, and shift it to
            // take into account the page bounds
            RectF margins = velocityBiasedMargins(horizontalBuffer, verticalBuffer, velocity);
            margins = shiftMarginsForPageBounds(margins, metrics);

            return getTileAlignedDisplayPortMetrics(margins, metrics);
        }

        public boolean aboutToCheckerboard(ImmutableViewportMetrics metrics, PointF velocity, DisplayPortMetrics displayPort) {
            // calculate the danger zone amounts based on the prefs
            float dangerZoneX = metrics.getWidth() * (DANGER_ZONE_BASE_X_MULTIPLIER + (velocity.x * DANGER_ZONE_INCR_X_MULTIPLIER));
            float dangerZoneY = metrics.getHeight() * (DANGER_ZONE_BASE_Y_MULTIPLIER + (velocity.y * DANGER_ZONE_INCR_Y_MULTIPLIER));
            // clamp it such that when added to the viewport, they don't exceed page size.
            // this is a prerequisite to calling shiftMarginsForPageBounds as we do below.
            dangerZoneX = Math.min(dangerZoneX, metrics.getPageWidth() - metrics.getWidth());
            dangerZoneY = Math.min(dangerZoneY, metrics.getPageHeight() - metrics.getHeight());

            // split the danger zone into margins based on velocity, and ensure it doesn't exceed
            // page bounds.
            RectF dangerMargins = velocityBiasedMargins(dangerZoneX, dangerZoneY, velocity);
            dangerMargins = shiftMarginsForPageBounds(dangerMargins, metrics);

            // we're about to checkerboard if the current viewport area + the danger zone margins
            // fall out of the current displayport anywhere.
            RectF adjustedViewport = new RectF(
                    metrics.viewportRectLeft - dangerMargins.left,
                    metrics.viewportRectTop - dangerMargins.top,
                    metrics.viewportRectRight + dangerMargins.right,
                    metrics.viewportRectBottom + dangerMargins.bottom);
            return !displayPort.contains(adjustedViewport);
        }

        @Override
        public String toString() {
            return "VelocityBiasStrategy mult=" + SIZE_MULTIPLIER + ", threshold=" + VELOCITY_THRESHOLD + ", reverse=" + REVERSE_BUFFER
                + ", dangerBaseX=" + DANGER_ZONE_BASE_X_MULTIPLIER + ", dangerBaseY=" + DANGER_ZONE_BASE_Y_MULTIPLIER
                + ", dangerIncrX=" + DANGER_ZONE_INCR_Y_MULTIPLIER + ", dangerIncrY=" + DANGER_ZONE_INCR_Y_MULTIPLIER;
        }
    }
}
