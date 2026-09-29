/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * JSDialog.rootCommentControl, JSDialog.commentControl - comment widgets
 */

declare var JSDialog: any;

const _createComment = function (container: HTMLElement, data: any) {
	// Create annotation copy and add it into the container.
	container.appendChild(data.annotation.sectionProperties.container);
	data.annotation.show();
	data.annotation.update();
	data.annotation.setExpanded();
};

const _emptyCommentWizard = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
) {
	window.L.DomUtil.addClass(parentContainer, 'content-has-no-comments');
	const emptyCommentWizard = window.L.DomUtil.create(
		'figure',
		'empty-comment-wizard-container',
		parentContainer,
	);
	const imgNode = window.L.DomUtil.create(
		'img',
		'empty-comment-wizard-img',
		emptyCommentWizard,
	);
	app.LOUtil.setImage(imgNode, 'lc_showannotations.svg', builder.map);
	imgNode.alt = data.text;

	const textNode = window.L.DomUtil.create(
		'figcaption',
		'empty-comment-wizard',
		emptyCommentWizard,
	);
	textNode.innerText = data.text;
	window.L.DomUtil.create('br', 'empty-comment-wizard', textNode);

	if (app.isCommentEditingAllowed()) {
		var linkNode = window.L.DomUtil.create(
			'div',
			'empty-comment-wizard-link',
			textNode,
		);
		linkNode.innerText = _('Insert Comment');
		linkNode.onclick = builder.map.insertComment.bind(builder.map);
	}
};

JSDialog.rootCommentControl = function (
	parentContainer: HTMLElement,
	data: any,
	builder: JSBuilder,
) {
	if (data.type === 'emptyCommentWizard') {
		_emptyCommentWizard(parentContainer, data, builder);
		return;
	}

	const mainContainerID =
		'explorable-entry level-' + builder._currentDepth + ' ' + data.id;
	let mainContainer = document.getElementById(mainContainerID);

	if (!mainContainer) {
		mainContainer = window.L.DomUtil.create(
			'div',
			'ui-explorable-entry level-' +
				builder._currentDepth +
				' ' +
				builder.options.cssClass,
			parentContainer,
		);
	}

	if (!mainContainer) {
		console.error(
			'ui-explorable-entry with id: ' + mainContainerID + ' does not exist!',
		);
		return;
	}
	const nonNullMainContainer = mainContainer;
	mainContainer.id = mainContainerID;

	let container = document.getElementById(data.id);
	if (!container) {
		container = window.L.DomUtil.create(
			'div',
			'ui-header cool-annotation-header level-' +
				builder._currentDepth +
				' ' +
				builder.options.cssClass +
				' ui-widget',
			mainContainer,
		);
	}

	if (!container) {
		console.error(
			'cool-annotation-header with id: ' + data.id + ' does not exist!',
		);
		return;
	}

	(container as any).annotation = data.annotation;
	container.id = data.id;
	_createComment(container, data);

	if (
		data.children.length > 1 &&
		mainContainer.id !== 'comment-thread' + data.id
	) {
		const numberOfReplies = data.children.length - 1;
		if (numberOfReplies > 0) {
			const replyCountNodeID = 'reply-count-node-' + data.id;
			let replyCountNode = document.getElementById(replyCountNodeID);
			if (!replyCountNode) {
				replyCountNode = window.L.DomUtil.create(
					'div',
					'cool-annotation-reply-count cool-annotation-content',
					container.querySelector(':scope .cool-annotation-content-wrapper'),
				);
			}

			if (!replyCountNode) {
				console.error(
					'cool-annotation-reply-count with id: ' +
						replyCountNodeID +
						' does not exist!',
				);
				return;
			}

			replyCountNode.id = 'reply-count-node-' + data.id;
			replyCountNode.style.display = 'block';

			const replyCountText =
				numberOfReplies === 1
					? numberOfReplies + ' ' + _('reply')
					: numberOfReplies + ' ' + _('replies');

			replyCountNode.textContent = replyCountText;
		}

		let childContainer = document.getElementById('comment-thread' + data.id);
		if (!childContainer) {
			childContainer = window.L.DomUtil.create(
				'div',
				'ui-content level-' +
					builder._currentDepth +
					' ' +
					builder.options.cssClass,
				mainContainer,
			);
		}

		if (!childContainer) {
			console.error(
				'ui-explorable-entry with id: ' + mainContainerID + ' does not exist!',
			);
			return;
		}
		const nonNullChildContainer = childContainer;
		childContainer.id = 'comment-thread' + data.id;
		childContainer.title = _('Comment');
		childContainer.style.display = 'none';

		if (builder.wizard) {
			const coolAnnotationMenubar = container.querySelectorAll(
				':scope .cool-annotation-menubar',
			) as NodeListOf<HTMLTableRowElement>;

			if (coolAnnotationMenubar.length > 0)
				coolAnnotationMenubar[0].style.display = 'none';

			let arrowSpan: HTMLSpanElement | null = container.querySelector(
				":scope [id='arrow span " + data.id + "']",
			);

			if (!arrowSpan) {
				arrowSpan = window.L.DomUtil.create(
					'span',
					'sub-menu-arrow',
					container.querySelector(':scope .cool-annotation-content-wrapper'),
				);
			}

			if (arrowSpan) {
				arrowSpan.style.display = 'block';
				arrowSpan.textContent = '>';
				arrowSpan.style.padding = '0px';
				arrowSpan.id = 'arrow span ' + data.id;
			}

			const annotationContainer = container.querySelector(
				':scope .cool-annotation',
			) as HTMLDivElement;
			annotationContainer.onclick = function () {
				builder.wizard.goLevelDown(nonNullMainContainer);
				nonNullChildContainer.style.display = 'block';
				if (!nonNullChildContainer.childNodes.length)
					builder.build(nonNullChildContainer, data.children, false);
			};

			const backButtonID = 'mobile-wizard-back';
			const backButton = document.getElementById(backButtonID);
			if (backButton) {
				backButton.onclick = function () {
					if (backButton.className !== 'close-button') {
						if (!nonNullMainContainer.childNodes.length)
							builder.build(nonNullMainContainer, data, false);
						if (data.type === 'rootcomment') {
							var temp = document.getElementById('comment-thread' + data.id);
							if (temp) temp.style.display = 'block';
						}
					}
				};
			} else {
				console.error('button with id: ' + backButtonID + ' does not exist!');
			}
		}
	}

	container
		.querySelector(':scope .cool-annotation')
		?.addEventListener('click', function () {
			const commentSection = app.sectionContainer.getSectionWithName(
				app.CSections.CommentList.name,
			) as cool.CommentSection;
			commentSection.highlightComment(data.annotation);
		});
	return false;
};

// eslint-disable-next-line no-unused-vars
JSDialog.commentControl = function (
	parentContainer: HTMLElement,
	data: any,
	_builder: JSBuilder,
) {
	_createComment(parentContainer, data);
	return false;
};
