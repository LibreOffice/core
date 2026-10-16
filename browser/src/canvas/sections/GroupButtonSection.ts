/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/// What a button beside the band of a group or a diagram is for.
type GroupButtonRole = 'diagram' | 'enter' | 'leave' | 'ungroup';

/*
	One button of the column that stands beside the band of a group or a diagram, while a view
	draws from the objects. It knows the object whose band it stands beside, what it is for, its
	place in the column counted from the top, and whether it may be pressed now. What a press does
	is the selection's business.
*/
class GroupButtonSection extends GenericButtonSection {
	static readonly namePrefix: string = 'GroupButton_';

	/// The size of a button and the space between two of them, in CSS pixels.
	static readonly size: number = 32;
	static readonly spaceBetween: number = 5;

	public readonly objectId: number;
	public readonly role: GroupButtonRole;
	private slot: number;
	private pressable: boolean = true;

	constructor(objectId: number, role: GroupButtonRole, slot: number) {
		super(
			GroupButtonSection.nameFor(objectId, role),
			GroupButtonSection.size,
			GroupButtonSection.size,
			'group-button group-button-' + role,
			'',
		);

		this.objectId = objectId;
		this.role = role;
		this.slot = slot;
		this.sectionProperties.lastInputEvent = null;
		this.getHTMLObject()?.setAttribute(
			'data-cooltip',
			GroupButtonSection.tooltipFor(role),
		);
	}

	/*
		The text shown while the pointer rests on the button, the one the toolbar shows for the
		same command, with its shortcut. The engine's command that opens a diagram has no text on
		this side, so it is given here.
	*/
	private static tooltipFor(role: GroupButtonRole): string {
		const command =
			role === 'enter'
				? '.uno:EnterGroup'
				: role === 'leave'
					? '.uno:LeaveGroup'
					: role === 'ungroup'
						? '.uno:FormatUngroup'
						: undefined;
		if (!command) return _('Edit Diagram');

		return JSDialog.ShortcutsUtil.getTooltipLabel(
			_UNO(command, 'presentation'),
			command,
		);
	}

	/// The name of the button for that role beside the band of that object.
	public static nameFor(objectId: number, role: GroupButtonRole): string {
		return GroupButtonSection.namePrefix + String(objectId) + '_' + role;
	}

	/// How far one button is below the one above it, in core pixels.
	public static step(): number {
		return (
			(GroupButtonSection.size + GroupButtonSection.spaceBetween) * app.dpiScale
		);
	}

	/// Takes the place in the column and whether the button may be pressed, and stands there.
	public standAt(slot: number, pressable: boolean): void {
		this.slot = slot;
		this.pressable = pressable;
		this.getHTMLObject()?.classList.toggle('group-button-off', !pressable);
		if (!pressable) this.getHTMLObject()?.classList.remove('hovered');
		this.follow();
	}

	/*
		Puts the button where it belongs now, beside the band as it stands. While the mouse drags
		the selection the button cannot be pressed, so it is not shown then, and it comes back
		beside the band where the drag left it.
	*/
	public follow(): void {
		const shown = !GraphicSelection.selectionSection?.isDragged();
		if (shown !== this.isSectionShown()) this.setShowSection(shown);
		if (!shown) {
			app.map.tooltip?.beginHide();
			return;
		}

		const at = this.calculatePositionPixel();

		this.setPosition(at[0], at[1]);
		this.adjustHTMLObjectPosition();
	}

	/*
		Where the button stands: clear of the band on its right, by the space between two buttons,
		and its place in the column down from the top of the band. Where the band is not there to
		be asked, the button stays where it stands.
	*/
	calculatePositionPixel(): Array<number> {
		const band = GraphicSelection.selectionSection?.bandBox(this.objectId);
		if (!band) return [this.position[0], this.position[1]];

		return [
			Math.round(
				band.maxX +
					HandleLook.reachOfASurround() +
					GroupButtonSection.spaceBetween * app.dpiScale,
			),
			Math.round(band.minY + this.slot * GroupButtonSection.step()),
		];
	}

	/*
		Shows the button where it comes into the part of the document in view, and hides it where
		it leaves it, also when it is moved there by its band rather than by the view scrolling,
		as a turned group's band growing and shrinking again takes it out of view and back.
	*/
	public onDocumentObjectVisibilityChange(): void {
		this.onNewDocumentTopLeft();
	}

	public onMouseEnter(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.pressable) super.onMouseEnter(point, e);
		app.map.tooltip?.beginShow(this.getHTMLObject());
	}

	public onMouseLeave(point: cool.SimplePoint, e: MouseEvent): void {
		super.onMouseLeave(point, e);
		app.map.tooltip?.beginHide();
	}

	public onRemove(): void {
		app.map.tooltip?.beginHide();
		super.onRemove();
	}

	public onClick(point: cool.SimplePoint, e: MouseEvent): void {
		if (this.pressable)
			GraphicSelection.pressTheGroupButton(this.objectId, this.role);
	}
}
