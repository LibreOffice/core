// @ts-strict-ignore
/* -*- tab-width: 4 -*- */

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * The animated element of the bitmap layer path. Its content is the raster of one shape,
 * delivered as its own slide layer. A frame draws it as a texture, or as an image on the 2d
 * fallback, in the element's current state.
 */

class AnimatedElementBitmap extends AnimatedElement {
	private animatedLayerInfo: AnimatedShapeInfo = null;
	private aLayer: ImageBitmap = null;
	private aBBoxInCanvas: BoundingBoxType;
	private aBaseElement: AnimatedObjectType;
	private tfContext: RenderContextGl;

	protected updateContentInfo() {
		const aAnimatedLayerInfo =
			this.aSlideShowContext.aSlideShowHandler.getAnimatedLayerInfo(
				this.slideHash,
				this.sId,
			);
		this.updateAnimationInfo(aAnimatedLayerInfo);
	}

	// The layer carries the visibility at slide start.
	protected getInitialVisibility(): boolean {
		return this.animatedLayerInfo ? this.animatedLayerInfo.initVisible : false;
	}

	private isGlSupported() {
		return !this.tfContext.is2dGl();
	}

	private createBaseElement(
		layer: ImageBitmap,
		bounds: BoundingBoxType,
	): AnimatedObjectType {
		if (!layer) {
			window.app.console.error(
				'AnimatedElementBitmap.createBaseElement: layer is null/undefined',
			);
			return null;
		}
		if (!bounds || bounds.width <= 0 || bounds.height <= 0) {
			window.app.console.error(
				`AnimatedElementBitmap.createBaseElement: invalid bounds: ${JSON.stringify(bounds)}`,
			);
			return null;
		}
		const canvas = new OffscreenCanvas(bounds.width, bounds.height);
		const context = canvas.getContext('2d');
		context.drawImage(
			layer,
			bounds.x,
			bounds.y,
			bounds.width,
			bounds.height,
			0,
			0,
			bounds.width,
			bounds.height,
		);
		return canvas.transferToImageBitmap();
	}

	public updateAnimationInfo(animatedLayerInfo: AnimatedShapeInfo) {
		if (!animatedLayerInfo) {
			// create a dummy bounding box for not break animation engine
			this.aBaseBBox = new DOMRect(0, 0, 1920, 1080);
			window.app.console.log(
				'AnimatedElementBitmap.updateAnimationInfo: passed info is not valid',
			);
			return;
		}
		this.animatedLayerInfo = animatedLayerInfo;

		this.aLayer = (this.animatedLayerInfo.content as ImageInfo).data;
		if (!this.aLayer) {
			window.app.console.log(
				'AnimatedElementBitmap.updateAnimationInfo: layer not valid',
			);
		}

		this.setBaseBBox(this.animatedLayerInfo.bounds);
		this.aBBoxInCanvas = convert(this.canvasScaleFactor, this.aBaseBBox);
		this.aBaseElement = this.createBaseElement(this.aLayer, this.aBBoxInCanvas);

		if (this.animatedLayerInfo.fillColor) {
			this.aBaseFillColor = colorParser(
				this.animatedLayerInfo.fillColor,
			) as RGBColor;
		}
		if (this.animatedLayerInfo.lineColor) {
			this.aBaseLineColor = colorParser(
				this.animatedLayerInfo.lineColor,
			) as RGBColor;
		}
		if (this.animatedLayerInfo.fontColor) {
			this.aBaseFontColor = colorParser(
				this.animatedLayerInfo.fontColor,
			) as RGBColor;
		}

		this.DBG(
			'.updateAnimationInfo: ' +
				`\n  aBaseBBox: ${BBoxToString(this.aBaseBBox)}` +
				`\n  aBBoxInCanvas: ${BBoxToString(this.aBBoxInCanvas)}` +
				`\n  base center: x: ${this.nBaseCenterX} y: ${this.nBaseCenterY}`,
		);

		this.bIsValid = true;
	}

	private getTextureFromElement(element: AnimatedObjectType) {
		if (!element || element.width <= 0 || element.height <= 0) {
			window.app.console.error(
				`AnimatedElementBitmap.getTextureFromElement: invalid element (null or zero-sized)`,
			);
			return null;
		}
		return this.tfContext.loadTexture(element);
	}

	public setTransitionParameters(transitionParameters: TransitionParameters) {
		if (!this.tfContext) {
			const layerCompositor = this.getLayerCompositor();
			if (!layerCompositor) {
				window.app.console.error(
					'AnimatedElementBitmap.setTransitionParameters: layer compositor is undefined',
				);
				return;
			}
			this.tfContext = layerCompositor.getLayerRendererContext();
			if (!this.tfContext) {
				window.app.console.error(
					'AnimatedElementBitmap.setTransitionParameters: layer renderer context is undefined',
				);
				return;
			}
		}

		transitionParameters.context = this.tfContext;
		transitionParameters.current = null;
		transitionParameters.next = this.getTextureFromElement(this.aBaseElement);
	}

	private getLayerCompositor(): SlideCompositor {
		const presenter: SlideShowPresenter = app.map.slideShowPresenter;
		if (presenter) {
			return presenter._slideCompositor;
		}
		return null;
	}

	applyTransitionFilters(
		properties?: AnimatedElementRenderProperties,
	): boolean {
		if (this.transitionFiltersManager.isEmpty() || !this.isGlSupported()) {
			return false;
		}
		return this.transitionFiltersManager.apply(properties);
	}

	public renderLayer(renderer: LayerRenderer) {
		if (!this.getVisibility()) return;
		if (renderer instanceof LayerRendererGl) this.renderLayerGl(renderer);
		else if (renderer instanceof LayerRenderer2d) this.renderLayer2d(renderer);
	}

	private renderLayer2d(renderer: LayerRenderer2d) {
		if (renderer.isDisposed()) return;

		const renderContext = renderer.getRenderContext();
		const renderingContext = renderContext.get2dOffscreen();

		renderingContext.save();

		// factor to convert from slide coordinate to canvas coordinate
		const scaleFactor = this.canvasScaleFactor;

		renderingContext.globalAlpha = this.getOpacity();

		const transformMatrix = this.getTransformMatrix();
		const transform = new DOMMatrix([
			transformMatrix.a,
			transformMatrix.b,
			transformMatrix.c,
			transformMatrix.d,
			transformMatrix.e * scaleFactor.x,
			transformMatrix.f * scaleFactor.y,
		]);
		renderingContext.setTransform(transform);

		const aElement = this.aBaseElement;
		app.console.debug(
			`AnimatedElementBitmap(${this.sId}).renderLayer2d:
				element width: ${aElement.width}
				element height: ${aElement.height}
				base center: (${this.nBaseCenterX}, ${this.nBaseCenterY})
				actual center: (${this.getX()}, ${this.getY()})
				transform: ${transform.toString()}`,
		);

		renderingContext.drawImage(
			aElement,
			0,
			0,
			aElement.width,
			aElement.height,
			this.aBBoxInCanvas.x,
			this.aBBoxInCanvas.y,
			aElement.width,
			aElement.height,
		);
		renderingContext.restore();
	}

	private renderLayerGl(renderer: LayerRendererGl) {
		if (renderer.isDisposed()) return;

		app.console.debug(
			`AnimatedElementBitmap(${this.sId}).renderLayerGl:
				element width: ${this.aBaseElement.width}
				element height: ${this.aBaseElement.height}
				base center: (${this.nBaseCenterX}, ${this.nBaseCenterY})
				actual center: (${this.getX()}, ${this.getY()})
				transform: ${this.getTransformMatrix().toString()}`,
		);

		const properties = this.getRenderProperties();

		if (this.applyTransitionFilters(properties)) return;

		renderer.drawBitmap(this.aBaseElement, properties);
	}
}

class AnimatedTextElement extends AnimatedElementBitmap {
	public isTextElement() {
		return true;
	}
	// TODO: to be implemented
}
